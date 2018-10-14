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

        bool updateChannel (const Channel* ch);

        void poll (int timeout_in_mills, std::vector<Channel*>& triggered_channels);

        bool removeChannel (const Channel* ch);

    private:
        bool registerChannel (const Channel* ch);
        bool modifyChannel (const Channel* ch);
        bool unregisterChannel (const Channel* ch);

        int _fd;
        std::unordered_map<int, Channel*> _channel_map;
};
