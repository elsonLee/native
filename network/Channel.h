#pragma once

#include <string>
#include <functional>

class EventLoop;

class Channel
{
    public:
        using CallbackFn = std::function<void()>;

        Channel (const std::string& name, int fd, EventLoop* event_loop);
        ~Channel ();

        Channel (const Channel&) = delete;
        Channel& operator= (const Channel&) = delete;

        const std::string& name () const { return _name; }
        int fd () const { return _fd; }
        int events () const { return _events; }
        void setRevents (int revents) { _revents = revents; }

        void handleEvent ();

        void setReadCallback  (const CallbackFn& read_cb)  { _read_cb  = read_cb; }
        void setWriteCallback (const CallbackFn& write_cb) { _write_cb = write_cb; }
        void setCloseCallback (const CallbackFn& close_cb) { _close_cb = close_cb; }
        void setErrorCallback (const CallbackFn& error_cb) { _error_cb = error_cb; }

        bool isReading () const { return _is_reading; }
        bool isWriting () const { return _is_writing; }

        bool enableReadEvent ();
        bool disableReadEvent ();
        bool isReadEventOn () const;

        bool enableWriteEvent ();
        bool disableWriteEvent ();
        bool isWriteEventOn () const;

        bool enableAllEvent ();
        bool disableAllEvent ();

    private:
        bool update ();

        const std::string _name;
        const int         _fd;
        EventLoop*        _event_loop;
        int               _events;
        int               _revents;

        bool              _is_reading;
        bool              _is_writing;

        CallbackFn        _read_cb;
        CallbackFn        _write_cb;
        CallbackFn        _close_cb;
        CallbackFn        _error_cb;
};
