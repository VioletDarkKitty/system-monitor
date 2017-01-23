/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "workerthread.h"

workerThread::workerThread()
{
    shouldQuit = false;
    shouldPause = false;
    workerThreadThread = nullptr;
}

workerThread::~workerThread()
{
    // wait for the thread to exit gracefully!
    quit();
    while(running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void workerThread::start()
{
    if (workerThreadThread!=nullptr) {
        throw std::exception();
    }

    workerThreadThread = new std::thread(
                &workerThread::run,this);
    workerThreadThread->detach();
}

void workerThread::quit()
{
    shouldQuit = true;
    shouldPause = false;
}

bool workerThread::running()
{
    return (workerThreadThread!=nullptr) && !shouldPause;
}

void workerThread::setPaused(bool pause)
{
    shouldPause = pause;
}

void workerThread::run()
{
    try {
        while(1) {
            loop();

            while(shouldPause) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (shouldQuit) {
                throw std::exception();
            }
        }
    } catch (std::exception &e) {
        shouldQuit = false;
        delete workerThreadThread;
        workerThreadThread = nullptr;
    }
}
