#pragma once

namespace liteshell
{
    /**
     * @brief The input stream manager
     *
     * This class manages the input stream for the shell. Typically, the input comes from stdin, but when reading from
     * a batch script, the input stream comes from the script file instead.
     */
    class InputStream
    {
    private:
        static const std::string ECHO_OFF;
        static const std::string ECHO_ON;
        static const std::string STREAM_EOF;

        std::list<std::string> _list;
        std::list<std::string>::iterator _iterator = _list.begin();

        InputStream(const InputStream &) = delete;
        InputStream &operator=(const InputStream &) = delete;

    public:
        /** @brief A flag indicating that `getline` must echo the input to stdout */
        static const int FORCE_STDOUT = 1 << 2;

        /** @brief A flag indicating that `getline` must read input from stdin */
        static const int FORCE_STDIN = 1 << 1;

        /** @brief A flag indicating that `getline` must read input from file */
        static const int FORCE_STREAM = 1 << 0;

        /** @brief The current echo state */
        bool echo = true;

        /**
         * @brief Construct a new `InputStream` object
         */
        InputStream() {}

        /** @brief The echo state after the next command */
        bool peek_echo()
        {
            if (peek() == ECHO_ON)
            {
                return true;
            }

            if (peek() == ECHO_OFF)
            {
                return false;
            }

            return echo;
        }

        /**
         * @brief Peek the next command in the stream.
         * @return The next command in the input stream, or `std::nullopt` if the stream
         * reaches EOF
         */
        std::optional<std::string> peek()
        {
            for (auto iter = _iterator; iter != _list.end(); iter++)
            {
                auto text = utils::strip(*iter);
                if (!text.empty())
                {
                    return text;
                }
            }

            return std::nullopt;
        }

        /**
         * @brief Read the next command
         *
         * @param prompt The prompt to display before reading
         * @param flags The flags to use when reading the command
         * @return The next command in the input stream
         */
        std::string getline(const std::string &prompt, const int flags)
        {
#ifdef DEBUG
            std::cout << "Received getline request, flags = " << flags << std::endl;
            std::cout << "Current input stream: " << _list << std::endl;
            std::cout << "Iterator position: " << std::distance(_list.begin(), _iterator);
            if (_iterator != _list.end())
            {
                std::cout << " (\"" << *_iterator << "\")";
            }
            std::cout << std::endl;
#endif

            if ((flags & FORCE_STDIN) & (flags & FORCE_STREAM))
            {
                throw std::invalid_argument("Arguments conflict: FORCE_STDIN && FORCE_STREAM");
            }

            if ((flags & FORCE_STREAM) && eof())
            {
                throw std::runtime_error("Unexpected EOF while reading");
            }

            if ((flags & FORCE_STDIN) || eof())
            {
                // Read from stdin
                while (true)
                {
                    if ((flags & FORCE_STDOUT) || (echo && peek_echo()))
                    {
                        std::cout << prompt << std::flush;
                    }

                    std::string line;
                    std::getline(std::cin, line);

                    line = utils::strip(line);
                    if (line == ECHO_OFF)
                    {
                        echo = false;
                        continue;
                    }
                    else if (line == ECHO_ON)
                    {
                        echo = true;
                        continue;
                    }
                    else if (line == STREAM_EOF)
                    {
                        clear();
                        continue;
                    }
                    else if (!line.empty() && line[0] == ':')
                    {
                        continue;
                    }
                    else if (std::cin.fail() || std::cin.eof() || line.empty())
                    {
                        std::cin.clear();
                        std::cout << std::endl;
                        continue;
                    }

#ifdef DEBUG
                    std::cout << "Response for getline request: " << line << std::endl;
#endif
                    return line;
                }
            }
            else
            {
                // Read from stream
                auto echo_state = echo && peek_echo();
                auto line = utils::strip(*_iterator++);
                if (line == ECHO_OFF)
                {
                    echo = false;
                    return getline(prompt, flags);
                }
                else if (line == ECHO_ON)
                {
                    echo = true;
                    return getline(prompt, flags);
                }
                else if (line == STREAM_EOF)
                {
                    clear();
                    if (flags & FORCE_STREAM)
                    {
                        throw std::runtime_error("Unexpected EOF while reading");
                    }

                    return getline(prompt, flags);
                }
                else if (!line.empty() && line[0] == ':')
                {
                    return getline(prompt, flags);
                }
                else if (echo_state)
                {
                    std::cout << prompt << line << std::endl;
                }

#ifdef DEBUG
                std::cout << "Response for getline request: " << line << std::endl;
#endif
                return line;
            }
        }

        template <typename _ForwardIterator>
        void write(const _ForwardIterator &__begin, const _ForwardIterator &__end)
        {
            _iterator = _list.insert(_iterator, __begin, __end);
        }

        void write(const std::string &data)
        {
            std::vector<std::string> lines;
            for (auto &line : utils::split(data, '\n'))
            {
                line = utils::strip(line);
                if (!line.empty())
                {
                    lines.push_back(line);
                }
            }

            write(lines.begin(), lines.end());
        }

        void clear()
        {
#ifdef DEBUG
            std::cout << "Clearing input stream" << std::endl;
#endif

            _list.erase(_list.begin(), _iterator);

#ifdef DEBUG
            std::cout << "Cleared input stream, current size = " << _list.size() << std::endl;
#endif
        }

        /** @brief Whether this stream reaches EOF */
        bool eof() const
        {
            return _iterator == _list.end();
        }

        void append_footer(std::stringstream &stream)
        {
            stream << "\n";
            stream << STREAM_EOF << "\n";
            stream << (echo ? ECHO_ON : ECHO_OFF) << "\n";
        }

        /** @brief Jump to the specified label */
        void jump(const std::string &label)
        {
            for (auto iter = _iterator; iter != _list.end(); iter++)
            {
                if (utils::strip(*iter) == label)
                {
                    _iterator = iter;
#ifdef DEBUG
                    std::cout << "Shift iterator to position " << std::distance(_list.begin(), _iterator) << std::endl;
#endif
                    return;
                }
            }

            for (auto iter = _list.begin(); iter != _iterator; iter++)
            {
                if (utils::strip(*iter) == label)
                {
                    _iterator = iter;
#ifdef DEBUG
                    std::cout << "Shift iterator to position " << std::distance(_list.begin(), _iterator) << std::endl;
#endif
                    return;
                }
            }

            throw std::runtime_error(utils::format("Label \"%s\" not found", label.c_str()));
        }
    };

    const std::string InputStream::ECHO_ON = "@ON";
    const std::string InputStream::ECHO_OFF = "@OFF";
    const std::string InputStream::STREAM_EOF = ":EOF";
}
