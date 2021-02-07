#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] {return !_queue.empty();});
    T message = std::move(_queue.front());
    _queue.pop_front();
    return message;
 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.emplace_back(msg);
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto message = _queue.receive();
        if (message == TrafficLightPhase::green)
        {
            return;
        }
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{

threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    //cycle duration: rand value between 4-6 seconds
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> distr (4000, 6000); //ms
    int cycleDuration = distr(eng); // duration of a single simulation cycle in ms

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            //set current Phase of Traffic Light to toggle 
         if (_currentPhase == TrafficLightPhase::red) 
         {
             _currentPhase = TrafficLightPhase::green;
         }
         else
         {
             _currentPhase = TrafficLightPhase::red;
         }
         //send message
         TrafficLightPhase message = TrafficLight::getCurrentPhase();
         _queue.send(std::move(message));

         cycleDuration = distr(eng);
         
         lastUpdate = std::chrono::system_clock::now();
    

        }
    }
    
    
}

