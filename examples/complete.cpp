/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#include <cli/clilocalsession.h> // include boost asio
#include <cli/remotecli.h>
// TODO. NB: remotecli.h and clilocalsession.h both includes boost asio,
// so in Windows it should appear before cli.h that include rang
// (consider to provide a global header file for the library)
#include <cli/cli.h>
#include <cli/filehistorystorage.h>

#define ENABLE_TELNET_SERVER

using namespace cli;
using namespace std;

int main()
{
    ASIO_SERVICE ios;
    CmdHandler colorCmd;
    CmdHandler nocolorCmd;

    // setup cli

    auto rootMenu = make_unique< Menu >( "cli" );
    rootMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, world\n"; },
            "Print hello world" );
    rootMenu -> Insert(
            "hello_everysession",
            [](std::ostream&){ Cli::cout() << "Hello, everybody" << std::endl; },
            "Print hello everybody on all open sessions" );
    rootMenu -> Insert(
            "answer",
            [](std::ostream& out, int x){ out << "The answer is: " << x << "\n"; },
            "Print the answer to Life, the Universe and Everything" );
    rootMenu -> Insert(
            "file",
            [](std::ostream& out, int fd)
            {
                out << "file descriptor: " << fd << "\n";
            },
            "Print the file descriptor specified",
            {"file_descriptor"} );
    rootMenu -> Insert(
            "echo", {"string to echo"},
            [](std::ostream& out, const string& arg)
            {
                out << arg << "\n";
            },
            "Print the string passed as parameter" );
    rootMenu -> Insert(
            "echo", {"first string to echo", "second string to echo"},
            [](std::ostream& out, const string& arg1, const string& arg2)
            {
                out << arg1 << ' ' << arg2 << "\n";
            },
            "Print the strings passed as parameter" );
    rootMenu -> Insert(
            "error",
            [](std::ostream&)
            {
                throw std::logic_error("Error in cmd");
            },
            "Throw an exception in the command handler" );
    rootMenu -> Insert(
            "reverse", {"string_to_revert"},
            [](std::ostream& out, const string& arg)
            {
                string copy(arg);
                std::reverse(copy.begin(), copy.end());
                out << copy << "\n";
            },
            "Print the reverse string" );
    rootMenu -> Insert(
            "add", {"first_term", "second_term"},
            [](std::ostream& out, int x, int y)
            {
                out << x << " + " << y << " = " << (x+y) << "\n";
            },
            "Print the sum of the two numbers" );
    rootMenu -> Insert(
            "add",
            [](std::ostream& out, int x, int y, int z)
            {
                out << x << " + " << y << " + " << z << " = " << (x+y+z) << "\n";
            },
            "Print the sum of the three numbers" );
    colorCmd = rootMenu -> Insert(
            "color",
            [&](std::ostream& out)
            {
                out << "Colors ON\n";
                SetColor();
                colorCmd.Disable();
                nocolorCmd.Enable();
            },
            "Enable colors in the cli" );
    nocolorCmd = rootMenu -> Insert(
            "nocolor",
            [&](std::ostream& out)
            {
                out << "Colors OFF\n";
                SetNoColor();
                colorCmd.Enable();
                nocolorCmd.Disable();
            },
            "Disable colors in the cli" );
    rootMenu->Insert(
            "removecmds",
            [&](std::ostream&)
            {
                colorCmd.Remove();
                nocolorCmd.Remove();
            }
    );

    auto subMenu = make_unique< Menu >( "sub" );
    subMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, submenu world\n"; },
            "Print hello world in the submenu" );
    subMenu -> Insert(
            "demo",
            [](std::ostream& out){ out << "This is a sample!\n"; },
            "Print a demo string" );

    auto subSubMenu = make_unique< Menu >( "subsub" );
        subSubMenu -> Insert(
            "hello",
            [](std::ostream& out){ out << "Hello, subsubmenu world\n"; },
            "Print hello world in the sub-submenu" );
    subMenu -> Insert( std::move(subSubMenu));

    rootMenu -> Insert( std::move(subMenu) );

    // create a cli with the given root menu and a persistent storage
    // you must pass to FileHistoryStorage the path of the history file
    // if you don't pass the second argument, the cli will use a VolatileHistoryStorage object that keeps in memory
    // the history of all the sessions, until the cli is shut down.
    Cli cli( std::move(rootMenu), std::make_unique<FileHistoryStorage>(".cli") );
    // global exit action
    cli.ExitAction( [](std::ostream & out){ out << "Goodbye and thanks for all the fish.\n"; } );
    // std exception custom handler
    cli.StdExceptionHandler(
        [](std::ostream& out, const std::string& cmd, const std::exception& e)
        {
            out << "Exception caught in cli handler: "
                << e.what()
                << " handling command: "
                << cmd
                << ".\n";
        }
    );

    CliLocalTerminalSession localSession(cli, ios, std::cout, 200);
    localSession.ExitAction(
        [&ios](std::ostream & out) // session exit action
        {
            out << "Closing App...\n";
            ios.stop();
        }
    );

    // setup server

#ifdef ENABLE_TELNET_SERVER

    CliTelnetServer server(ios, 5000, cli);
    // exit action for all the connections
    server.ExitAction( [](std::ostream & out) { out << "Terminating this session...\n"; } );

#else // ENABLE_TELNET_SERVER

#ifdef CLI_OLD_ASIO
    ASIO_NS::io_service::work work(ios);
#else
    auto work = ASIO_NS::make_work_guard(ios);
#endif  

#endif // ENABLE_TELNET_SERVER

    ios.run();

    return 0;
}
