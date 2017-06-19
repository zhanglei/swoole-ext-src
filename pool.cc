/*
  +----------------------------------------------------------------------+
  | Swoole                                                               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

#include "phpx.h"
#include "swoole.h"

#include <iostream>
#include <queue>

using namespace php;

struct Pool
{
    queue<Variant> taskQueue;
    queue<Variant> idlePool;
    unordered_map<int, Object> resourcePool;
    Variant create_function;
    size_t size;
    size_t resourceNum;
    size_t failureCount;
};

PHPX_METHOD(Pool, __construct)
{
    if (args.count() < 2)
    {
        error(E_ERROR, "2 arguments are required");
        return;
    }

    _this.set("config", args[0]);
    _this.set("size", args[1]);

    Pool *pool = new Pool;
    pool->size = args[0].toInt();
    _this.store(pool);
}

PHPX_METHOD(Pool, __destruct)
{
    Pool *pool = _this.fetch<Pool>();
    while (!pool->taskQueue.empty())
    {
        Variant task = pool->taskQueue.front();
        task.delRef();
        pool->taskQueue.pop();
    }
    while (!pool->idlePool.empty())
    {
        Variant res = pool->idlePool.front();
        res.delRef();
        pool->idlePool.pop();
    }
    _this.store<Pool>(nullptr);
}

PHPX_METHOD(Pool, join)
{
    Pool *pool = _this.fetch<Pool>();
    Object o = args[0];
    pool->resourcePool[o.getId()] = o;
    _this.exec("release", o);
}

PHPX_METHOD(Pool, failure)
{
    Pool *pool = _this.fetch<Pool>();
    pool->resourceNum --;
    pool->failureCount ++;
}

PHPX_METHOD(Pool, create)
{
    auto cb = args[0];
    if (!cb.isCallable())
    {
        error(E_WARNING, "parameter 1 is not callable.");
        return;
    }
    Pool *pool = _this.fetch<Pool>();
    pool->create_function = cb;
}

PHPX_METHOD(Pool, remove)
{
    Object o = args[0];
    Pool *pool = _this.fetch<Pool>();
    int rid = o.getId();
    if (!pool)
    {
        return;
    }
    if (pool->resourcePool.find(rid) == pool->resourcePool.end())
    {
        retval = false;
    }
    else
    {
        pool->resourcePool.erase(rid);
        pool->resourceNum--;
    }
}

PHPX_METHOD(Pool, request)
{
    Pool *pool = _this.fetch<Pool>();
    auto cb = args[0];
    if (!cb.isCallable())
    {
        error(E_WARNING, "parameter 1 is not callable.");
        return;
    }
    cb.addRef();
    pool->taskQueue.push(cb);
    if (pool->idlePool.size() > 0)
    {
        _this.exec("doTask");
    }
    else if (pool->resourcePool.size() < pool->size && pool->resourceNum < pool->size)
    {
        call(pool->create_function);
        pool->resourceNum++;
    }
}

PHPX_METHOD(Pool, release)
{
    Pool *pool = _this.fetch<Pool>();
    pool->idlePool.push(args[0]);
    if (pool->taskQueue.size() > 0)
    {
        _this.exec("doTask");
    }
}

PHPX_METHOD(Pool, doTask)
{
    Pool *pool = _this.fetch<Pool>();
    Object resource;
    while (!pool->idlePool.empty())
    {
        Object res = pool->idlePool.front();
        pool->idlePool.pop();
        int rid = res.getId();
        if (pool->resourcePool.find(rid) == pool->resourcePool.end())
        {
            continue;
        }
        else
        {
            resource = res;
            break;
        }
    }
    if (resource.isNull())
    {
        if (pool->resourcePool.empty())
        {
            call(pool->create_function);
            pool->resourceNum++;
        }
        return;
    }
    Variant task = pool->taskQueue.front();
    pool->taskQueue.pop();
    Args _arg_list;
    _arg_list.append(resource);
    call(task, _arg_list);
    task.delRef();
    resource.delRef();
}
