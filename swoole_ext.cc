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

using namespace php;

PHPX_METHOD(Pool, __construct);
PHPX_METHOD(Pool, __destruct);
PHPX_METHOD(Pool, join);
PHPX_METHOD(Pool, failure);
PHPX_METHOD(Pool, create);
PHPX_METHOD(Pool, remove);
PHPX_METHOD(Pool, request);
PHPX_METHOD(Pool, release);
PHPX_METHOD(Pool, doTask);

PHPX_METHOD(RedisPool, __construct);
PHPX_METHOD(RedisPool, connect);
PHPX_METHOD(RedisPool, __call);

PHPX_METHOD(FunctionObject, call)
{
    auto call = _this.get("_call");
    Array params = _this.get("_params");

    for (int i = 0; i < args.count(); i++)
    {
        params.append(args[i]);
    }

    php::call(call, params);
}

PHPX_EXTENSION()
{
    Extension *ext = new Extension("swoole_ext", "0.0.1");
    ext->require("swoole");

    ext->onStart = [ext]() noexcept
    {
        ext->registerConstant("SWOOLE_EXT_VERSION", ext->version);

        Class *c = new Class("Swoole\\Pool");
        c->alias("swoole_pool");
        c->addMethod(PHPX_ME(Pool, __construct), CONSTRUCT);
        c->addMethod(PHPX_ME(Pool, __destruct), DESTRUCT);
        c->addMethod(PHPX_ME(Pool, join));
        c->addMethod(PHPX_ME(Pool, release));
        c->addMethod(PHPX_ME(Pool, create));
        c->addMethod(PHPX_ME(Pool, remove));
        c->addMethod(PHPX_ME(Pool, failure));
        c->addMethod(PHPX_ME(Pool, request));
        c->addMethod(PHPX_ME(Pool, doTask));
        ext->registerClass(c);

        Class *rp = new Class("Swoole\\RedisPool");
        rp->extends("Swoole\\Pool");
        rp->addMethod(PHPX_ME(RedisPool, __construct), CONSTRUCT);
        rp->addMethod(PHPX_ME(RedisPool, connect));

        ArgInfo *argInfo = new ArgInfo(2);
        argInfo->add("query");
        argInfo->add("callback");
        rp->addMethod(PHPX_ME(RedisPool, __call), PUBLIC, argInfo);
        rp->addConstant("DEFAULT_PORT", 6379);
        ext->registerClass(rp);

        Class *fo = new Class("FunctionObject");
        fo->addMethod(PHPX_ME(FunctionObject, call));
        ext->registerClass(fo);
    };

    ext->info(
    {
        "swoole_ext support", "enabled"
    },
    {
        { "author", "Rango" },
        { "version", ext->version },
        { "date", "2017-05-22" },
    });

    return ext;
}
