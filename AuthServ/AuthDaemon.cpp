/******************************************************************************
 * This file is part of dirtsand.                                             *
 *                                                                            *
 * dirtsand is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * dirtsand is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with dirtsand.  If not, see <http://www.gnu.org/licenses/>.          *
 ******************************************************************************/

#include "AuthServer_Private.h"
#include "NetIO/MsgChannel.h"
#include "settings.h"
#include "errors.h"
#include <unistd.h>

pthread_t s_authDaemonThread;
DS::MsgChannel s_authChannel;

void dm_auth_init()
{
    //
}

void dm_auth_shutdown()
{
    pthread_mutex_lock(&s_authClientMutex);
    std::list<AuthServer_Private*>::iterator client_iter;
    for (client_iter = s_authClients.begin(); client_iter != s_authClients.end(); ++client_iter)
        DS::CloseSock((*client_iter)->m_sock);
    pthread_mutex_unlock(&s_authClientMutex);

    bool complete = false;
    for (int i=0; i<50 && !complete; ++i) {
        pthread_mutex_lock(&s_authClientMutex);
        size_t alive = s_authClients.size();
        pthread_mutex_unlock(&s_authClientMutex);
        if (alive == 0)
            complete = true;
        usleep(100000);
    }
    if (!complete)
        fprintf(stderr, "[Auth] Clients didn't die after 5 seconds!");

    pthread_mutex_destroy(&s_authClientMutex);
}

void* dm_authDaemon(void*)
{
    for ( ;; ) {
        try {
            DS::FifoMessage msg = s_authChannel.getMessage();
            switch (msg.m_messageType) {
            case DS::e_AuthShutdown:
                dm_auth_shutdown();
                return 0;
            default:
                /* Invalid message...  This shouldn't happen */
                DS_DASSERT(0);
                break;
            }
        } catch (DS::SockHup) {
            // Client wasn't paying attention anyway...
        }
    }

    dm_auth_shutdown();
    return 0;
}

void DS::AuthDaemon_SendMessage(int msg, void* data)
{
    s_authChannel.putMessage(msg, data);
}