/**
 * @file
 *
 * This file tests AllJoyn use of the DBus wire protocol
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <ctype.h>
#include <qcc/platform.h>

#include <qcc/Debug.h>
#include <qcc/Util.h>

#include <alljoyn/MsgArg.h>
#include <alljoyn/version.h>
#include <Status.h>

using namespace qcc;
using namespace std;
using namespace ajn;

/* BYTE */
static uint8_t y = 0;
/* BOOLEAN */
static bool b = true;
/* INT16 */
static int16_t n = 42;
/* UINT16 */
static uint16_t q = 0xBEBE;
/* DOUBLE */
static double d = 3.14159265L;
/* INT32 */
static int32_t i = -9999;
/* INT32 */
static uint32_t u = 0x32323232;
/* INT64 */
static int64_t x = -1LL;
/* UINT64 */
static uint64_t t = 0x6464646464646464ULL;
/* STRING */
static const char* s = "this is a string";
/* OBJECT_PATH */
static const char* o = "/org/foo/bar";
/* SIGNATURE */
static const char* g = "a{is}d(siiux)";

/* Array of BYTE */
static uint8_t ay[] = { 9, 19, 29, 39, 49 };
/* Array of INT16 */
static int16_t an[] = { -9, -99, 999, 9999 };
/* Array of INT32 */
static int32_t ai[] = { -8, -88, 888, 8888 };
/* Array of INT64 */
static int64_t ax[] = { -8, -88, 888, 8888 };
/* Array of UINT64 */
static int64_t at[] = { -8, -88, 888, 8888 };
/* Array of DOUBLE */
static double ad[] = { 0.001, 0.01, 0.1, 1.0, 10.0, 100.0 };
/* Array of STRING */
static const char* as[] = { "one", "two", "three", "four" };
/* Array of OBJECT_PATH */
static const char* ao[] = { "/org/one", "/org/two", "/org/three", "/org/four" };
/* Array of SIGNATURE */
static const char* ag[] = { "s", "sss", "as", "a(iiiiuu)" };

int main(int argc, char** argv)
{
    QStatus status = ER_OK;

    printf("AllJoyn Library version: %s\n", ajn::GetVersion());
    printf("AllJoyn Library build info: %s\n", ajn::GetBuildInfo());

    /*
     * Test cases using MsgArg constructors
     */
    if (status == ER_OK) {
        MsgArg arg("i", 1);
        uint32_t i;
        status = arg.Get("i", &i);
    }
    if (status == ER_OK) {
        MsgArg arg("s", "hello");
        char* str;
        status = arg.Get("s", &str);
    }
    /*
     * Basic types
     */
    if (status == ER_OK) {
        MsgArg argList;
        status = argList.Set("(ybnqdiuxtsoqg)", y, b, n, q, &d, i, u, &x, &t, s, o, q, g);
        if (status == ER_OK) {
            status = argList.Get("(ybnqdiuxtsoqg)", &y, &b, &n, &q, &d, &i, &u, &x, &t, &s, &o, &q, &g);
        }
    }
    /*
     * Structs
     */
    if (status == ER_OK) {
        MsgArg argList;
        status = argList.Set("((ydx)(its))", y, &d, &x, i, &t, s);
        if (status == ER_OK) {
            status = argList.Get("((ydx)(its))", &y, &d, &x, &i, &t, &s);
        }
    }
    if (status == ER_OK) {
        MsgArg arg;
        status = arg.Set("((iuiu)(yd)at)", i, u, i, u, y, &d, ArraySize(at), at);
        if (status == ER_OK) {
            int64_t* p64;
            size_t p64len;
            status = arg.Get("((iuiu)(yd)at)", &i, &u, &i, &u, &y, &d, &p64len, &p64);
        }
    }
    /*
     * Variants
     */
    if (status == ER_OK) {
        for (uint8_t n = 0; n < 3; ++n) {
            MsgArg arg;

            switch (n) {
            case 0:
                arg.Set("v", new MsgArg("i", i));
                break;

            case 1:
                arg.Set("v", new MsgArg("d", &d));
                break;

            case 2:
                arg.Set("v", new MsgArg("s", "hello world"));
                break;
            }

            int32_t i;
            double d;
            char* str;
            status = arg.Get("i", &i);
            if (status == ER_BUS_SIGNATURE_MISMATCH) {
                status = arg.Get("s", &str);
                if (status == ER_BUS_SIGNATURE_MISMATCH) {
                    status = arg.Get("d", &d);
                }
            }
            if (status != ER_OK) {
                break;
            }
        }
    }
    /*
     * Arrays of scalars
     */
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ay", ArraySize(ay), ay);
        uint8_t* pay;
        size_t lay;
        status = arg.Get("ay", &lay, &pay);
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("an", ArraySize(an), an);
        uint16_t* pan;
        size_t lan;
        status = arg.Get("an", &lan, &pan);
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ai", ArraySize(ai), ai);
        int32_t* pai;
        size_t lai;
        status = arg.Get("ai", &lai, &pai);
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ax", ArraySize(ax), ax);
        int64_t* pax;
        size_t lax;
        status = arg.Get("ax", &lax, &pax);
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ad", ArraySize(ad), ad);
        double* pad;
        size_t lad;
        status = arg.Get("ad", &lad, &pad);
    }
    /*
     * Arrays of various string types. These all unpack as MsgArg arrays.
     */
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("as", ArraySize(as), as);
        MsgArg* asArray;
        size_t las;
        status = arg.Get("as", &las, &asArray);
        if (status == ER_OK) {
            for (size_t i = 0; i < ArraySize(as); ++i) {
                char* pas;
                status = asArray[i].Get("s", &pas);
                if (status != ER_OK) {
                    break;
                }
                if (strcmp(as[i], pas) != 0) {
                    status = ER_FAIL;
                    break;
                }
            }
        }
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ag", ArraySize(ag), ag);
        MsgArg* agArray;
        size_t lag;
        status = arg.Get("ag", &lag, &agArray);
        if (status == ER_OK) {
            for (size_t i = 0; i < ArraySize(ag); ++i) {
                char* pag;
                status = agArray[i].Get("g", &pag);
                if (status != ER_OK) {
                    break;
                }
                if (strcmp(ag[i], pag) != 0) {
                    status = ER_FAIL;
                    break;
                }
            }
        }
    }
    if (status == ER_OK) {
        MsgArg arg;
        arg.Set("ao", ArraySize(ao), ao);
        MsgArg* aoArray;
        size_t lao;
        status = arg.Get("ao", &lao, &aoArray);
        if (status == ER_OK) {
            for (size_t i = 0; i < lao; ++i) {
                char* pao;
                status = aoArray[i].Get("o", &pao);
                if (status != ER_OK) {
                    break;
                }
                if (strcmp(ao[i], pao) != 0) {
                    status = ER_FAIL;
                    break;
                }
            }
        }
    }
    /*
     * Variant of Empty array of strings
     */
    if (status == ER_OK) {
        MsgArg vArg;
        MsgArg arg;
        arg.Set("as", 0, NULL);
        vArg.Set("v", &arg);
        MsgArg* variant;
        MsgArg* asArray;
        size_t las;
        status = vArg.Get("v", &variant);
        if (status == ER_OK) {
            status = variant->Get("as", &las, &asArray);
            if (las != 0) {
                status = ER_FAIL;
            }
        }
    }
    /*
     * Dictionary
     */
    if (status == ER_OK) {

        const char* keys[] = { "red", "green", "blue", "yellow" };

        MsgArg dict(ALLJOYN_ARRAY);
        size_t numEntries = ArraySize(keys);
        MsgArg* dictEntries = new MsgArg[ArraySize(keys)];

        dictEntries[0].Set("{iv}", 0, new MsgArg("s", keys[0]));
        dictEntries[1].Set("{iv}", 1, new MsgArg("(ss)", keys[1], "bean"));
        dictEntries[2].Set("{iv}", 2, new MsgArg("s", keys[2]));
        dictEntries[3].Set("{iv}", 3, new MsgArg("(ss)", keys[3], "mellow"));

        status = dict.v_array.SetElements("{iv}", numEntries, dictEntries);
        if (status == ER_OK) {
            MsgArg* entries;
            size_t num;
            status = dict.Get("a{iv}", &num, &entries);
            if (status == ER_OK) {
                for (size_t i = 0; i < num; ++i) {
                    char* str1;
                    char* str2;
                    uint32_t key;
                    status = entries[i].Get("{is}", &key, &str1);
                    if (status == ER_BUS_SIGNATURE_MISMATCH) {
                        status = entries[i].Get("{i(ss)}", &key, &str1, &str2);
                    }
                    if (key != i) {
                        status = ER_FAIL;
                    }
                    if (status != ER_OK) {
                        break;
                    }
                }
            }
        }
        /* Try using wilcard matching */
        if (status == ER_OK) {
            MsgArg* entries;
            size_t num;
            status = dict.Get("a{iv}", &num, &entries);
            if (status == ER_OK) {
                for (size_t i = 0; i < num; ++i) {
                    MsgArg* val;
                    uint32_t key;
                    status = entries[i].Get("{iv}", &key, &val);
                    if (key != i) {
                        status = ER_FAIL;
                    }
                    if (status != ER_OK) {
                        break;
                    }
                }
            }
        }
    }

    if (status == ER_OK) {
        QCC_SyncPrintf("\nPASSED\n");
    } else {
        QCC_SyncPrintf("\nFAILED %s\n", QCC_StatusText(status));
    }

    return 0;
}
