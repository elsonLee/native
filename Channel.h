#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

class EventLoop;

class Channel
{
    public:
        using CallbackFn = std::function<void()>;

        Channel (int fd, EventLoop* event_loop);
        ~Channel ();

        Channel (const Channel&) = delete;
        Channel& operator= (const Channel&) = delete;

        int fd () const { return _fd; }
        int events () const { return _events; }
        void setRevents (int revents) { _revents = revents; }

        void handleEvent ();

        void setReadCallback  (const CallbackFn& read_cb)  { _read_cb  = read_cb; }
        void setWriteCallback (const CallbackFn& write_cb) { _write_cb = write_cb; }
        void setCloseCallback (const CallbackFn& close_cb) { _close_cb = close_cb; }
        void setErrorCallback (const CallbackFn& error_cb) { _error_cb = error_cb; }

        bool enableReadEvent ();
        bool disableReadEvent ();

        bool enableWriteEvent ();
        bool disableWriteEvent ();
        bool isWriteEventOn ();

        bool enableAllEvent ();
        bool disableAllEvent ();

    private:
        bool update ();

        const int   _fd;
        EventLoop*  _event_loop;
        int         _events;
        int         _revents;
        CallbackFn  _read_cb;
        CallbackFn  _write_cb;
        CallbackFn  _close_cb;
        CallbackFn  _error_cb;
};

#endif // CHANNEL_H
