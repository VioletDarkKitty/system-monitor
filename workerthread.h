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
#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H
#include <thread>

class workerThread
{
public:
    workerThread();
    virtual ~workerThread();
    void quit();
    void start();
    bool running();
    void setPaused(bool pause);    
private:
    bool shouldQuit;
    bool shouldPause;
    void run();
    virtual void loop(){}
    std::thread* workerThreadThread;
};

#endif // WORKERTHREAD_H
