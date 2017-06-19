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

#include "pool.h"

using namespace php;

PHPX_METHOD(RedisPool, __construct)
{
    if (args.count() < 2)
    {
        error(E_ERROR, "must take exactly 2 arguments");
        return;
    }
    auto config = args[0];
    auto size = args[1];

    Array arr(config);
    if (!arr.exists("host"))
    {
        throwException("Exception", "require redis host option.");
        return;
    }
    if (!arr.exists("port"))
    {
        arr.set("port", DEFAULT_PORT_REDIS);
    }

    _this.callParentMethod("__construct", config, size);
    Array callback;
    callback.append(_this);
    callback.append("connect");
    _this.exec("create", callback);
}

PHPX_METHOD(RedisPool, onClose)
{
    _this.exec("remove", args[0]);
}

PHPX_METHOD(RedisPool, onConnect)
{
    auto redis = args[0];
    auto result = args[1];

    if (result.isEmpty())
    {
        _this.exec("failure");
//        error(E_WARNING, "connect to redis server[%s:%p] failed. Error: %s[%p].");
    }
    else
    {

    }
}

PHPX_METHOD(RedisPool, onFinish)
{
    auto redis = args[2];
    auto params = args[1];
    auto call = args[0];

    Array func;
    func.append(redis);
    func.append(call);

    Args _arg_list;
    _arg_list.append(func);
    _arg_list.append(params);

    php::call("call_user_func_array", _arg_list);
    _this.exec("release", redis);
}

PHPX_METHOD(RedisPool, connect)
{
    Object redis = newObject("swoole_redis");
    Array callback;
    callback.append(_this);
    callback.append("onClose");
    redis.exec("on", callback);

    Array config = _this.get("config");
    auto host = config.get("host");
    auto port = config.get("port");

    Array callback2;
    callback2.append(_this);
    callback2.append("onConnect");
    redis.exec("connect", callback2);
}

PHPX_METHOD(RedisPool, __call)
{
    Array _call;
    _call.append(_this);
    _call.append("onFinish");

    Array _params;
    _params.append(args[0]);
    _params.append(args[1]);

    Object func = newObject("FunctionObject");
    func.set("_call", _call);
    func.set("_params", _params);

    Array callback;
    callback.append(func);
    callback.append("call");

    _this.exec("request", callback);
}
