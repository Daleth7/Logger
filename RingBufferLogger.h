#ifndef RINGBUFFER_H
#define RINGBUFFER_H

/*
    A basic ring buffer logger for general use.

    It requires that you set its output destination,
    which may be the console or file, so long as it
    is a child of std::ostream. If the output destination
    is not specified, it is unsafe to call dump().

    This logger also allows you to specified the maximum
    number of logs to hold before dumping and whether or
    not to always dump.
*/

#include <queue>
#include <string>
#include <cstddef>
#include <ostream>
#include <stdexcept>

template <typename message>
class RingBufferLogger{
    public:
    //Type aliases
        using buffer    = std::queue<message>;
        using str_type  = std::string;
        using outptr    = std::ostream*;
        using size_type = std::size_t;

    // Log levels
        enum class level : unsigned char{
            Unspecified = 0,
            General,
            Warning,
            Fatal
        };

    //Read-only
            //Maximum number of logs to store before dumping
        size_type max() const;
            //Address of output destination. This return nullptr if
            //  a file path is used.
        outptr output() const;
            //File path of destination file. This returns an empty
            //  string if an output address is used.
        str_type filepath() const;
            //Dump logs with each log.
        bool always_dump() const;

    //Settings modifiers
            //The following two functions allow the caller to optionally
            //  retrieve the old file path or output address.
        outptr set_output(outptr, str_type* = nullptr);
        str_type set_filepath(const str_type&, outptr = nullptr);
        str_type set_filepath(str_type&&, outptr = nullptr);
        void set_max(size_type);
        void set_always_dump(bool);

    //Log with a log level
        RingBufferLogger& log(level, const message&);
        template <typename T>
        RingBufferLogger& log(level, const T&);
            //Log more than one entry
        template <typename T, typename T2, typename... TN>
        RingBufferLogger& log(level, const T&, const T2&, const TN&...);
            //Take all input and collate into one log entry
        template <typename... T>
        RingBufferLogger& log_collect(level, const T&...);

    //Log without specifying log level
        RingBufferLogger& log(const message&);
        template <typename T>
        RingBufferLogger& log(const T&);
        template <typename T, typename T2, typename... TN>
        RingBufferLogger& log(const T&, const T2&, const TN&...);
        template <typename... T>
        RingBufferLogger& log_collect(const T&...);
            //Write all the logs to the output destination. Allow the user
            //  to specify how the information is written.
        RingBufferLogger& dump(std::ios_base::openmode = std::ios_base::app)
            throw(std::runtime_error);
        RingBufferLogger& clear();

        template <typename T>
        RingBufferLogger<message>& operator<<(const T&);

    //Constructors and destructor
            //Pass the address of the output destination.
        RingBufferLogger(outptr, size_type = m_default_max, bool=false);
            //Construct an output file instead.
        RingBufferLogger(const str_type&, size_type = m_default_max, bool=false);

        RingBufferLogger(const RingBufferLogger&)               = delete;
        RingBufferLogger(RingBufferLogger&&)                    = delete;
        RingBufferLogger& operator=(const RingBufferLogger&)    = delete;
        RingBufferLogger& operator=(RingBufferLogger&&)         = delete;
        ~RingBufferLogger();

    protected:
    //Helpers
        template <typename T>
        message log_collect_helper(const T&);
        template <typename T, typename T2, typename... TN>
        message log_collect_helper(const T&, const T2&, const TN&...);

        void get_local_time(char[], size_type);

    private:
        buffer      m_buffer;
        size_type   m_max;
        str_type    m_filepath;
        outptr      m_output;
        bool        m_always_dump;

        static constexpr size_type m_default_max = 1e3;
};

#include "RingBufferLogger.inl"

typedef RingBufferLogger<std::string> RBLogger;

    //Log the entry with information on where it originated
#define rblog(RBOBJ, ...)   \
    (RBOBJ).log_collect(__func__, "(): ", __VA_ARGS__)

#define rblog_level(RBOBJ, LEVEL, ...)   \
    (RBOBJ).log_collect(LEVEL, __func__, "(): ", __VA_ARGS__)

#endif