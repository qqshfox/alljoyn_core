/**
 * @file
 * @brief Sample implementation of an AllJoyn client.
 *
 * This client will subscribe to the nameChanged signal sent from the 'org.alljoyn.Bus.signal_sample'
 * service.  When a name change signal is sent this will print out the new value for the
 * 'name' property that was sent by the service.
 *
 */

/******************************************************************************
 * Copyright 2010-2011, Qualcomm Innovation Center, Inc.
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

#include <qcc/platform.h>

#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <vector>

#include <qcc/String.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/version.h>
#include <alljoyn/AllJoynStd.h>

#include <Status.h>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;
using namespace ajn;

/*constants*/
static const char* INTERFACE_NAME = "org.alljoyn.Bus.signal_sample";
static const char* SERVICE_NAME = "org.alljoyn.Bus.signal_sample";
static const char* SERVICE_PATH = "/";

/** Static top level message bus object */
static BusAttachment* g_msgBus = NULL;

static bool s_joinComplete = false;

/** Signal handler */
static void SigIntHandler(int sig)
{
    if (NULL != g_msgBus) {
        QStatus status = g_msgBus->Stop(false);
        if (ER_OK != status) {
            printf("BusAttachment::Stop() failed\n");
        }
    }
    exit(0);
}

/** AlljounListener receives discovery events from AllJoyn */
class MyBusListener : public BusListener {
  public:

    MyBusListener() : BusListener(), sessionId(0) { }

    void FoundAdvertisedName(const char* name, const QosInfo& advQos, const char* namePrefix)
    {
        if (0 == strcmp(name, SERVICE_NAME)) {
            printf("FoundAdvertisedName(name=%s, prefix=%s)\n", name, namePrefix);
            /* We found a remote bus that is advertising basic sercice's  well-known name so connect to it */
            uint32_t disposition;
            QosInfo qos;
            QStatus status = g_msgBus->JoinSession(name, disposition, sessionId, qos);
            if ((ER_OK != status) || (ALLJOYN_JOINSESSION_REPLY_SUCCESS != disposition)) {
                printf("JoinSession failed (status=%s, disposition=%d)\n", QCC_StatusText(status), disposition);
            } else {
                printf("JoinSession SUCCESS (Session id=%d)\n", sessionId);
            }
        }
        s_joinComplete = true;
    }

    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME))) {
            printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }

    SessionId GetSessionId() const { return sessionId; }

  private:
    SessionId sessionId;
};

/** Static bus listener */
static MyBusListener g_busListener;

class SignalListeningObject : public BusObject {
  public:
    SignalListeningObject(BusAttachment& bus, const char* path) :
        BusObject(bus, path),
        nameChangedMember(NULL)
    {
        /* Add org.alljoyn.Bus.signal_sample interface */
        InterfaceDescription* intf = NULL;
        QStatus status = bus.CreateInterface(INTERFACE_NAME, intf);
        if (status == ER_OK) {
            printf("Interface created successfully.\n");
            intf->AddSignal("nameChanged", "s", "newName", 0);
            intf->AddProperty("name", "s", PROP_ACCESS_RW);
            intf->Activate();
        } else {
            printf("Failed to create interface %s\n", INTERFACE_NAME);
        }

        status = AddInterface(*intf);

        if (status == ER_OK) {
            printf("Interface successfully added to the bus.\n");
            /* Register the signal handler 'nameChanged' with the bus*/
            nameChangedMember = intf->GetMember("nameChanged");
            assert(nameChangedMember);
        } else {
            printf("Failed to Add interface: %s", INTERFACE_NAME);
        }

        /* register the signal handler for the the 'nameChanged' signal */
        status =  bus.RegisterSignalHandler(this,
                                            static_cast<MessageReceiver::SignalHandler>(&SignalListeningObject::NameChangedSignalHandler),
                                            nameChangedMember,
                                            NULL);
        if (status != ER_OK) {
            printf("Failed to register signal handler for %s.nameChanged\n", SERVICE_NAME);
        } else {
            printf("Registered signal handler for %s.nameChanged\n", SERVICE_NAME);
        }
        /* Empty constructor */
    }

    QStatus SubscribeNameChangedSignal(void) {
        return bus.AddMatch("type='signal',interface='org.alljoyn.Bus.signal_sample',member='nameChanged'");
    }

    void NameChangedSignalHandler(const InterfaceDescription::Member* member,
                                  const char* sourcePath,
                                  Message& msg)
    {
        printf("--==## signalConsumer: Name Changed signal Received ##==--\n");
        printf("\tNew name: %s\n", msg->GetArg(0)->v_string.str);

    }

  private:
    const InterfaceDescription::Member* nameChangedMember;

};

/** Main entry point */
int main(int argc, char** argv, char** envArg)
{
    QStatus status = ER_OK;

    printf("AllJoyn Library version: %s\n", ajn::GetVersion());

    /* Install SIGINT handler */
    signal(SIGINT, SigIntHandler);

    const char* connectArgs = getenv("BUS_ADDRESS");
    if (connectArgs == NULL) {
#ifdef _WIN32
        connectArgs = "tcp:addr=127.0.0.1,port=9955";
#else
        connectArgs = "unix:abstract=alljoyn";
#endif
    }

    g_msgBus = new BusAttachment("myApp", true);

    /* Start the msg bus */
    status = g_msgBus->Start();
    if (ER_OK != status) {
        printf("BusAttachment::Start failed\n");
    } else {
        printf("BusAttachment started.\n");
    }

    /* Connect to the bus */
    if (ER_OK == status) {
        status = g_msgBus->Connect(connectArgs);
        if (ER_OK != status) {
            printf("BusAttachment::Connect(\"%s\") failed\n", connectArgs);
        } else {
            printf("BusAttchement connected to %s\n", connectArgs);
        }
    }

    SignalListeningObject object(*g_msgBus, SERVICE_PATH);
    /* Register object */
    g_msgBus->RegisterBusObject(object);


    /* Register a bus listener in order to get discovery indications */
    g_msgBus->RegisterBusListener(g_busListener);
    printf("BusListener Registered.\n");
    if (ER_OK == status) {
        uint32_t disposition = 0;
        status = g_msgBus->FindAdvertisedName(SERVICE_NAME, disposition);
        if ((status != ER_OK) || (disposition != ALLJOYN_FINDADVERTISEDNAME_REPLY_SUCCESS)) {
            printf("org.alljoyn.Bus.FindAdvertisedName failed (%s) (disposition=%d)\n", QCC_StatusText(status), disposition);
            status = (status == ER_OK) ? ER_FAIL : status;
        }
    }

    /* Wait for join session to complete */
    while (!s_joinComplete) {
#ifdef _WIN32
        Sleep(10);
#else
        sleep(1);
#endif
    }

    if (status == ER_OK) {
        status = object.SubscribeNameChangedSignal();
        if (status != ER_OK) {
            printf("Failed to Subscribe to the Name Changed Signal.\n");
        } else {
            printf("Successfully Subscribed to the Name Changed Signal.\n");
        }
    }

    if (status == ER_OK) {
        /*
         * Wait until bus is stopped
         */
        g_msgBus->WaitStop();
    } else {
        printf("BusAttachment::Start failed\n");
    }

    /* Deallocate bus */
    if (g_msgBus) {
        BusAttachment* deleteMe = g_msgBus;
        g_msgBus = NULL;
        delete deleteMe;
    }

    printf("Exiting with status %d (%s)\n", status, QCC_StatusText(status));

    return (int) status;
}
