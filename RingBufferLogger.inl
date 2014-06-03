#include <fstream>
#include <ctime>
#include <sstream>

template <typename message>
auto RingBufferLogger<message>::max() const -> size_type
    {return m_max;}

template <typename message>
typename RingBufferLogger<message>::outptr
    RingBufferLogger<message>::output() const
{return m_output;}

template <typename message>
typename RingBufferLogger<message>::str_type
    RingBufferLogger<message>::filepath() const
{return m_filepath;}

template <typename message>
bool RingBufferLogger<message>::always_dump() const
    {return m_always_dump;}

template <typename message>
typename RingBufferLogger<message>::outptr
    RingBufferLogger<message>::set_output(
    outptr newout,
    str_type* request
){
    outptr toreturn(m_output);
    m_output = newout;
    if(request != nullptr) *request = m_filepath;
    m_filepath.clear();
    return toreturn;
}

template <typename message>
typename RingBufferLogger<message>::str_type
    RingBufferLogger<message>::set_filepath(
    const str_type& newfilepath,
    outptr request
){
    str_type toreturn(m_filepath);
    m_filepath = newfilepath;
    if(request != nullptr) request = m_output;
    m_output = nullptr;
    return toreturn;
}

template <typename message>
typename RingBufferLogger<message>::str_type
    RingBufferLogger<message>::set_filepath(
    str_type&& newfilepath,
    outptr request
){
    str_type toreturn(m_filepath);
    m_filepath = newfilepath;
    if(request != nullptr) request = m_output;
    m_output = nullptr;
    return toreturn;
}

template <typename message>
void RingBufferLogger<message>::set_max(size_type newmax)
    {m_max = newmax;}

template <typename message>
void RingBufferLogger<message>::set_always_dump(bool newdumpstatus)
    {m_always_dump = newdumpstatus;}

template <typename message>
RingBufferLogger<message>&
    RingBufferLogger<message>::log(level log_level, const message& entry)
{
    if(m_buffer.size() == m_max) m_buffer.pop();

    const size_type buffer_size(100);
    char buffer[buffer_size]{};
    get_local_time(buffer, buffer_size);

    message level_image;
    switch(log_level){
        case level::General:    level_image = "(General) "; break;
        case level::Warning:    level_image = "(Warning) "; break;
        case level::Fatal:      level_image = "( Fatal ) "; break;
        case level::Unspecified:
        default:                level_image = "(  N/A  ) "; break;
    }

    m_buffer.push(message(buffer)+level_image+entry);
    if(m_always_dump && m_buffer.size() == m_max) this->dump();
    return *this;
}

template <typename message>
template <typename T>
RingBufferLogger<message>&
    RingBufferLogger<message>::log(level log_level, const T& entry)
{
    std::ostringstream oss;
    oss << entry;
    return this->log(log_level, oss.str());
}

template <typename message>
template <typename T, typename T2, typename... TN>
RingBufferLogger<message>& RingBufferLogger<message>::log(
    level log_level,
    const T& val,
    const T2& val2,
    const TN&... valn
){
    this->log(log_level, val);
    return this->log(log_level, val2, valn...);
}

template <typename message>
template <typename... T>
RingBufferLogger<message>&
    RingBufferLogger<message>::log_collect(level log_level, const T&... val)
{return this->log(log_level, this->log_collect_helper(val...));}

template <typename message>
RingBufferLogger<message>&
    RingBufferLogger<message>::log(const message& entry)
{return this->log(level::Unspecified, entry);}

template <typename message>
template <typename T>
RingBufferLogger<message>& RingBufferLogger<message>::log(const T& entry)
    {return this->log(level::Unspecified, entry);}

template <typename message>
template <typename T, typename T2, typename... TN>
RingBufferLogger<message>& RingBufferLogger<message>::log(
    const T& val,
    const T2& val2,
    const TN&... valn
){
    this->log(level::Unspecified, val);
    return this->log(level::Unspecified, val2, valn...);
}

template <typename message>
template <typename... T>
RingBufferLogger<message>&
    RingBufferLogger<message>::log_collect(const T&... val)
{return this->log(level::Unspecified, this->log_collect_helper(val...));}

#include <iomanip>
template <typename message>
RingBufferLogger<message>&
    RingBufferLogger<message>::dump(std::ios_base::openmode flags)
    throw(std::runtime_error)
{
    if(m_buffer.size() == 0) return *this;
    if(m_output != nullptr){
        while(m_buffer.size() > 0){
            *m_output << m_buffer.front();
            m_buffer.pop();
            *m_output << '\n';
        }
        *m_output << '\n' << std::setfill('-') << std::setw(70) << '\n';
    }else if(m_filepath != "N/A"){
        std::ofstream ofile(m_filepath, flags);
        if(!ofile.is_open()){
            this->log(message(__func__) + "(): " + "Unable to open file.");
            m_filepath = "N/A";
            return *this;
        }
        while(m_buffer.size() > 0){
            ofile << m_buffer.front();
            m_buffer.pop();
            ofile << '\n';
        }
        ofile << '\n' << std::setfill('-') << std::setw(70) << '\n';
        ofile.close();
    }else throw std::runtime_error("No dump location specified.");
    return *this;
}

template <typename message>
RingBufferLogger<message>& RingBufferLogger<message>::clear()
    {return m_buffer = buffer(), *this;}

template <typename message>
template <typename T>
RingBufferLogger<message>& RingBufferLogger<message>::operator<<(const T& val)
    {return this->log(val);}

template <typename message>
RingBufferLogger<message>::RingBufferLogger(
    outptr newout,
    size_type newmax,
    bool newdumpstatus
)
    : m_buffer()
    , m_max(newmax)
    , m_filepath("")
    , m_output(newout)
    , m_always_dump(newdumpstatus)
{}

template <typename message>
RingBufferLogger<message>::RingBufferLogger(
    const str_type& newfilepath,
    size_type newmax,
    bool newdumpstatus
)
    : m_buffer()
    , m_max(newmax)
    , m_filepath(newfilepath)
    , m_output(nullptr)
    , m_always_dump(newdumpstatus)
{}

template <typename message>
RingBufferLogger<message>::~RingBufferLogger()
    {this->dump();}

template <typename message>
template <typename T>
auto RingBufferLogger<message>::log_collect_helper(const T& val) -> message
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

template <typename message>
template <typename T, typename T2, typename... TN>
auto RingBufferLogger<message>::log_collect_helper(
    const T& val,
    const T2& val2,
    const TN&... valn
) -> message
{return this->log_collect_helper(val)
    + " " + this->log_collect_helper(val2, valn...);}

template <typename message>
void RingBufferLogger<message>::get_local_time(
    char buffer[],
    size_type buffer_size
){
    std::time_t raw;
    std::time(&raw);
    auto localt(std::localtime(&raw));
    std::strftime(buffer, buffer_size, "[%c] ", localt);
}