
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2013, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 2.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#ifndef INCLUDE_UTIL_COMMANDLINE_HPP
#define INCLUDE_UTIL_COMMANDLINE_HPP

#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

//*****************************************
// Command line parsing utility

namespace RCF {

    class I_CommandLineOption {
    public:
        virtual ~I_CommandLineOption() {}
        virtual void notify_begin() = 0;
        virtual void notify( std::string) = 0;
        virtual void notify_end() = 0;
        virtual std::string getName() = 0;
        virtual std::string getDefaultValue() = 0;
        virtual std::string getHelpString() = 0;
    };

    class CommandLine
    {
    private:
        CommandLine() 
        {}

    public:
        static CommandLine &getSingleton()
        {
            static CommandLine commandLine;
            return commandLine;
        }

        void parse(int argc, char **argv, bool exitOnHelp = true)
        {
            parse(argc, const_cast<const char **>(argv), exitOnHelp);
        }

        void parse(int argc, const char **argv, bool exitOnHelp = true)
        {
            mOptionValues.clear();
            mArgs.clear();
            int i=1;

            // If there is no "help" command line option registered, and the only command line option found is "help",
            // then we print out the helpstrings for all known command line options, and exit.
            if (argc == 2 && isKey(argv[1]) && toKey(argv[1]) == "help") 
            {
                if (mOptions.find( "help" ) == mOptions.end()) 
                {
                    std::cout << "Available command line switches:\n";
                    for (OptionIterator it = mOptions.begin(); it != mOptions.end(); it++) 
                    {
                        if ((*it).second) 
                        {
                            I_CommandLineOption *option = (*it).second;
                            std::cout << "-" << option->getName() << "\n";
                            std::cout << "\tDescription: " << option->getHelpString() << "\n";
                            std::cout << "\tDefault: " << option->getDefaultValue() << "\n";
                        }
                    }
                    if (exitOnHelp)
                    {
                        exit(0);
                    }
                }
            }

            // Parse the command line
            while (i < argc) 
            {
                std::string arg1 = argv[i];
                i++;
                std::string arg2 = (i<argc) ? argv[i] : "";
                i++;
                if (isKey(arg1)) 
                {
                    if (!isKey(arg2) )
                    {
                        mOptionValues[ toKey(arg1) ].push_back( arg2 );
                    }
                    else 
                    {
                        mOptionValues[ toKey(arg1) ].push_back( "" );
                        i--;
                    }
                    if (mOptions.find( toKey(arg1) ) == mOptions.end()) 
                    {
                        // This isn't very useful.
                        //std::cout << "Unrecognized option \"" << arg1 << "\"; type \"-help\" to list all options.\n";
                    }
                }
                else 
                {
                    mArgs.push_back( arg1 );
                    i--;
                }
            }

            // Notify the registered I_CommandLineOption objects
            for (OptionIterator iter = mOptions.begin(); iter != mOptions.end(); iter++)
            {
                if ((*iter).second)
                {
                    (*iter).second->notify_begin();
                }
            }

            for (OptionValueIterator iter = mOptionValues.begin(); iter != mOptionValues.end(); iter++) 
            {
                OptionIterator jter = mOptions.find((*iter).first);
                if (jter != mOptions.end())
                {
                    for (ValueIterator kter = (*iter).second.begin(); kter != (*iter).second.end(); kter++)
                    {
                        jter->second->notify( *kter );
                    }
                }
            }

            for (OptionIterator iter = mOptions.begin(); iter != mOptions.end(); iter++)
            {
                if ((*iter).second)
                {
                    (*iter).second->notify_end();
                }
            }
        }
     
        void registerOption(I_CommandLineOption *option)
        {
            mOptions[ option->getName() ] = option;
        }

        void unregisterOption(I_CommandLineOption *option)
        {
            OptionIterator it = mOptions.find( option->getName() );
            if (it != mOptions.end() && (*it).second == option)
            {
                mOptions.erase( it );
            }
        }
       
        void clear()
        {
            mOptions.clear();
            mOptionValues.clear();
            mArgs.clear();
        }
       
        template<typename T>
        T get(std::string name)
        {
            T t;
            lexical_cast( mOptionValues[name][0], t );
            return t;
        }
       
        const std::vector<std::string> &getArguments()
        {
            return mArgs;
        }
       
        const std::vector<std::string> &getValues(std::string name)
        {
            return mOptionValues[name];
        }
       
        template<typename T>
        T lexical_cast(std::string strValue, T* = NULL)
        {
            T t;
            //t = boost::lexical_cast<T, std::string>(strValue);
            lexical_cast(strValue, t);
            return t;
        }

    private:

        std::map< std::string, I_CommandLineOption *> mOptions;
        std::map<std::string, std::vector<std::string> > mOptionValues;
        std::vector<std::string> mArgs;
       
        typedef std::map< std::string, I_CommandLineOption *>::iterator OptionIterator;
        typedef std::map<std::string, std::vector<std::string> >::iterator OptionValueIterator;
        typedef std::vector<std::string>::iterator ValueIterator;

        bool isKey(const std::string &arg)
        {
            bool startsWithDash = (arg.size() > 1 && arg[0] == '-' );
            bool startsWithDoubleDash = (arg.size() > 2 && arg[0] == '-' && arg[1] == '-');
            return startsWithDash || startsWithDoubleDash;
        }

        std::string toKey(const std::string &arg)
        {
            assert( isKey(arg) );

            bool startsWithDash = (arg.size() > 1 && arg[0] == '-' );
            bool startsWithDoubleDash = (arg.size() > 2 && arg[0] == '-' && arg[1] == '-');

            if (startsWithDoubleDash)
            {
                return arg.substr(2);
            }
            else if (startsWithDash)
            {
                return arg.substr(1);
            }
            else
            {
                assert(0 && "invalid command line option syntax");
                return "";
            }
        }

        void lexical_cast( const std::string &strValue, bool &value )
        {
            if (strValue == "1" || strValue == "true" || strValue == "")
                value = true;
            else
                value = false;
        }
       
        void lexical_cast( const std::string &strValue, int &value )
        {
            value = atoi(strValue.c_str());
        }

        void lexical_cast( const std::string &strValue, unsigned int &value )
        {
            value = static_cast<unsigned int>(atoi(strValue.c_str()));
        }

        void lexical_cast( const std::string &strValue, std::string &value )
        {
            value = strValue;
        }

    };

    template<typename T>
    class CommandLineOption : public I_CommandLineOption {
    public:

        CommandLineOption(std::string name, T default_value, std::string helpString) : 
            name(name), default_value(default_value), helpString(helpString)
        {
            CommandLine::getSingleton().registerOption(this);
        }

        ~CommandLineOption()
        {
            CommandLine::getSingleton().unregisterOption(this);
        }
       
        operator T() const
        {
            return get();
        }
       
        //const std::vector<std::string> &getValues() const
        const std::vector<T> &getValues() const
        {
            return values;
        }
       
        T get() const
        {
            return (values.empty()) ? default_value : values[0];
        }
       
        void set(T t)
        {
            values.clear();
            values.push_back( t );
        }

        virtual void on_notify_end()
        {}
       
    private:

        void notify_begin()
        {
            values.clear();
        }

        void notify_end()
        {
            on_notify_end();
        }

        void notify( std::string value )
        {
            //values.push_back( CommandLine::getSingleton().template lexical_cast<T>(value) );
            values.push_back( CommandLine::getSingleton().lexical_cast(value, (T *) 0));
        }
       
        std::string getName()
        {
            return name;
        }
       
        std::string getHelpString()
        {
            return helpString;
        }

        std::string getDefaultValue()
        {
            std::ostringstream ostr;
            ostr << default_value ;
            return ostr.str();
        }

    private:
        std::string name;
        T default_value;
        std::vector<T> values;
        std::string helpString;
    };

    template<typename T>
    inline std::istream &operator>>(std::istream &is, CommandLineOption<T> &option)
    {
        T t;
        is >> t;
        option.set(t);
        return is;
    }

    template<typename T>
    inline RCF::MemOstream &operator<<(RCF::MemOstream &os, CommandLineOption<T> &option)
    {
        os << option.get();
        return os;
    }

} // namespace RCF

#endif // ! INCLUDE_UTIL_COMMANDLINE_HPP
