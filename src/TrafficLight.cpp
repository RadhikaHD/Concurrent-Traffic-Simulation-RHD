#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    // perform vector modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T t = std::move(_queue.back());
    _queue.pop_back();

    return t; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards
    //send a notification.
    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "   Object will be added to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}

/* Implementation of class "TrafficLight" */

void TrafficLight::setCurrentPhase(TrafficLightPhase phase)
{
    this->_currentPhase = phase;
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        auto color = trafficlightmessages.receive();
        if (color == green)
            return;
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when
    //the public method „simulate“ is called. To do this, use the thread queue in the base class.
    std::thread cyclethroughthread(&TrafficLight::cycleThroughPhases, this);
    TrafficObject::threads.emplace_back(std::move(cyclethroughthread));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random
    // value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    auto start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto timenow = std::chrono::high_resolution_clock::now();
        auto timepassed = std::chrono::duration_cast<std::chrono::milliseconds>(timenow - start);

        if (timepassed.count() >= rand() % 3000 + 4000)
        {

            //toggles the current phase of the traffic light between red and green
            if (this->_currentPhase == red)
            {
                this->_currentPhase = green;
                trafficlightmessages.send(green);
                std::cout<<"light is green"<<std::endl;
            }
            else
            {
                this->_currentPhase = red;
                trafficlightmessages.send(red);
                std::cout<<"light is red"<<std::endl;
            }
            // sends an update method to the message queue using move semantics
            //trafficlightmessages.send(std::move(_currentPhase));
            start = std::chrono::high_resolution_clock::now();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
