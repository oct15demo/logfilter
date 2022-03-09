<h3>logfilter</h3>

1) Stream filtering in C/C++
2) Logging and logging with stream filtering
3) Python wrapper of C/C++ in Eclipse with pydev to produce source link in logging console output
4) Sending a callback function from Python and incorparting it into stream filter
5) Some miscelaneous string notes, and use of fmt::format    
  
Demo of automatic stream filtering in C/C++,
output to standard out, cout, or ostream, can automatically be sent through a filter. The filter could filter or transform the input. In the demo it merely outputs input to the console with a prepended identifier, Filter a, or Filter b.
Logging employs a logging library https://github.com/gabime/spdlog . The demo shows logging also being sent through the filter.

Eclipse CDT, the C/C++ IDE does not apparently support log console output linking to source code, a familiar feature in Java. However, calling the C/C++ program from a python wrapper will produce the linking output, utilizing the pydev Eclipse plugin. 

While we're at it, we can send a python callback function to the C/C++ program which can also be called by the stream filter, which is how the python callback is used in this demo.

Finally, there is some miscellaneous string manipulation mixed in, including a note on fmt::format which allows type safe string formatting similar to python:<br>
  `"This is the python use of format Hello {}.".format("World")`<br>
In C/C++ fmt::format<br>
    `fmt::format("This is a demo of fmt::format Hello {}.","World");`<br>
fmt::format is included in the spdlog lib, so that similar syntax can be used in log messages. An equivalent to fmt::format is supposed to be part of c++20 as std::fmt but it does not seem to have been implemented anywhere yet. 

This code has been run on Mac OS High Sierra 10.13.1, compiled with -std=c++2a for c++20, with libs fmt and spdlog in Eclipse 4.7.3a, CDT plugin 9.4.3, pydev 7.3.0 

$ g++ --version<br>
Configured with: --prefix=/Library/Developer/CommandLineTools/usr --with-gxx-include-dir=/usr/include/c++/4.2.1
Apple LLVM version 10.0.0 (clang-1000.10.44.4)
