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
