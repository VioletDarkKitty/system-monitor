/*
 * Copyright (C) 2017 Lily Rivers (VioletDarkKitty)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3
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

/**
 * @brief workerThread::start Start the thread
 */
void workerThread::start()
{
    if (workerThreadThread!=nullptr) {
        throw std::exception();
    }

    workerThreadThread = new std::thread(
                &workerThread::run,this);
    workerThreadThread->detach();
}

/**
 * @brief workerThread::quit Mark the thread as quiting
 * The thread may take some time to exit after this is done, please wait until running shows false
 */
void workerThread::quit()
{
    shouldQuit = true;
    shouldPause = false;
}

/**
 * @brief workerThread::running Return if the thread is running and not going to pause next loop
 * @return If the thread is running
 */
bool workerThread::running()
{
    return (workerThreadThread!=nullptr) && !shouldPause;
}

/**
 * @brief workerThread::setPaused Set the pause state for the thread
 * @param pause If the thread should be paused when it next loops
 */
void workerThread::setPaused(bool pause)
{
    shouldPause = pause;
}

/**
 * @brief workerThread::run Begin looping this thread
 * Extended classes are expected to have a loop function to run
 * Blocking functions should be kept to a minimum in derived threads as quit will only set flags not throw
 */
void workerThread::run()
{
    try {
        while(1) {
            loop();

            while(shouldPause) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (shouldQuit) {
                /// TODO: Use a better exception, other functions may throw
                throw std::exception();
            }
        }
    } catch (std::exception &e) {
        shouldQuit = false;
        delete workerThreadThread;
        workerThreadThread = nullptr;
    }
}
