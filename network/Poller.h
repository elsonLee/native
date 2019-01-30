#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

class Channel;

class Poller
{
    public:

        Poller ();

        ~Poller ();

        void poll (int timeout_in_mills, std::vector<Channel*>& triggered_channels);

        bool updateChannel (const Channel* ch);

        bool removeChannel (const Channel* ch);

    private:

        bool createOrChangeEvent (int op, int event_fd, uint32_t events);

        bool removeEvent (int event_fd);

        bool registerChannel (const Channel* ch);

        bool modifyChannel (const Channel* ch);

        bool unregisterChannel (const Channel* ch);

        int _fdm

        std::unordered_map<int, Channel*> _channel_map;
};
