//
// Created by ganz on 28/07/17.
//

#ifndef DEMO_DLLOADER_HPP
#define DEMO_DLLOADER_HPP

#include <string>
#include <typeinfo>
#ifdef __unix__
# include <dlfcn.h>
#elif _WIN32
# include <Windows.h>
#endif
#include <exception>
#include <iostream>
#include "types.hpp"
#include "futils.hpp"

namespace futils
{
#ifdef __unix__
    class       Dloader
    {
        void            *_handle;
        std::string     _path;
    public:
        Dloader(std::string const &path, int mode = RTLD_LAZY):
                _path(path)
        {
            std::cout << "Loading library in " << path << std::endl;
            _handle = dlopen(path.c_str(), mode);
            if (_handle == nullptr)
                throw std::runtime_error(dlerror());
        }

        template    <typename T, typename ...Args>
        T           *build(Args ...args)
        {
            std::string symbol = "build";
            auto func = (T *(*)(Args ...))(dlsym(_handle, symbol.c_str()));
            if (func == nullptr)
            {
                std::cerr << __FUNCTION__ << ": failed. Cannot find builder symbol "
                                             + symbol + " in " + _path << std::endl;
                return nullptr;
            }
            return func(args...);
        };

        template    <typename Ret, typename ...Args>
        Ret           execute(std::string const &symbol, Args ...args)
        {
            auto func = (Ret (*)(Args ...))(dlsym(_handle, symbol.c_str()));
            if (func == nullptr) {
                std::cerr << __FUNCTION__ << ": failed. Cannot find symbol "
                                             + symbol + " in " + _path << std::endl;
                throw std::runtime_error("Dynamic execution of " + symbol + " failed.");
            }
            return func(args...);
        };
    };
#elif __APPLE__
    class       Dloader
    {
        void            *_handle;
        std::string     _path;
    public:
        Dloader(std::string const &path, int mode = RTLD_LAZY):
                _path(path)
        {
            _handle = dlopen(path.c_str(), mode);
            if (_handle == nullptr)
                throw std::runtime_error("Cannot find " + path);
        }

        template    <typename T, typename ...Args>
        T           *build(Args ...args)
        {
            std::string symbol = "build";
            auto func = (T *(*)(Args ...))(dlsym(_handle, symbol.c_str()));
            if (func == nullptr)
            {
                std::cerr << __FUNCTION__ << ": failed. Cannot find builder symbol "
                                             + symbol + " in " + _path << std::endl;
                return nullptr;
            }
            return func(args...);
        };

        template    <typename Ret, typename ...Args>
        Ret           execute(std::string const &symbol, Args ...args)
        {
            auto func = (Ret (*)(Args ...))(dlsym(_handle, symbol.c_str()));
            if (func == nullptr) {
                std::cerr << __FUNCTION__ << ": failed. Cannot find symbol "
                                             + symbol + " in " + _path << std::endl;
                throw std::runtime_error("Dynamic execution of " + symbol + " failed.");
            }
            return func(args...);
        };
    };
#elif _WIN32
	class Dloader
	{
		HINSTANCE inst;
	public:
		Dloader(std::string const &, int mode = 0) {
			inst = LoadLibrary("./fender.dll");
			if (!inst)
				throw std::runtime_error("Could not load fender.dll");
		}

		template <typename T, typename ...Args>
		T *build(Args...args)
		{
			return nullptr;
		}

		template <typename Ret, typename ...Args>
		Ret execute(std::string const &, Args ...args)
		{
			return Ret();
		}
	};
#endif
}

#endif //DEMO_DLLOADER_HPP
