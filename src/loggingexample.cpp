#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <ostream>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/tweakme.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO;
#define SPDLOG_SHORT_LEVEL_NAMES { "T", "D", "I", "W", "E", "C", "O" }
//see spdlog/tweakme.h for short level names
using namespace std;

static shared_ptr<spdlog::logger> staticlogptr;

//---------------------- Filter a section ------------------------//

/*******************************************************************
 *
 *   Demo of cout redirect, can be used as a filter or transform
 *   Adapted from stackoverflow url below
 *   Text "Filter a :" prepended to output just to demo capability
 *   "Finished with Filter" output is only to mark filtering completed
 *   for purposes of demo. Output is a batch process, not continuous.
 *
 *******************************************************************/

//https://stackoverflow.com/questions/1881589/redirected-cout-stdstringstream-not-seeing-eol
streambuf*     oldbuf;
stringstream   redirectStream;

void redirectStdOut(){
	oldbuf  = cout.rdbuf( redirectStream.rdbuf() );
}

void resetStdOut(){
	cout.rdbuf(oldbuf);
}

static int printOut(){ // comments from stackoverflow unless marked CS
    // loop enter once for each line. Note: str does not include the '\n' character.
    string str;
    while(getline(redirectStream, str)) {
        fprintf(stdout,"Filter a : %s\n",str.c_str());
    }
    //https://stackoverflow.com/questions/12112259/how-to-reuse-stringstream
    redirectStream.clear(); // redirect must be cleared in order to reuse CS
    cout << "Finished with Filter a\n";
    getline(redirectStream, str);
    fprintf(stdout,"Filter a : %s\n",str.c_str());

    // In real life use RAII to do this. Simplified here for code clarity.
	//cout.rdbuf(oldbuf);  //separate reset function replaces CS
    return 1;
}

//------------------- End of Filter a section ---------------------//

//---------------------- Filter b section ------------------------//

/*******************************************************************
 *
 *   Demo of cout redirect, can be used as a filter or transform
 *   Adapted (copied) from stackoverflow url below
 *   Text "Filter b :" prepended to output just to demo capability
 *   Output is continuous, like a pipe.
 *
 *******************************************************************/

//https://stackoverflow.com/questions/12826751/c-execute-function-any-time-a-stream-is-written-to

typedef function<void(string)> function_type;

class functionbuf : public streambuf {
	private:

	typedef streambuf::traits_type traits_type;

    function_type d_function;

    char  d_buffer[1024];

    int overflow(int c) {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *this->pptr() = traits_type::to_char_type(c);
            this->pbump(1);
        }
        return this->sync()? traits_type::not_eof(c): traits_type::eof();
    }

    int sync() {
        if (this->pbase() != this->pptr()) {
            this->d_function(std::string(this->pbase(), this->pptr()));
            this->setp(this->pbase(), this->epptr());
        }
        return 0;
    }

	public:

    functionbuf(function_type const& function): d_function(function) {
        this->setp(this->d_buffer, this->d_buffer + sizeof(this->d_buffer) - 1);
    }
};

class ofunctionstream : private virtual functionbuf, public std::ostream {
	public:

    ofunctionstream(function_type const& function): functionbuf(function), std::ostream(static_cast<std::streambuf*>(this)) {
        this->flags(ios_base::unitbuf);
    }
};

//----- Section for Filter b  supfilter and python callback ------//

/*******************************************************************
 * subfilter_type is a function pointer type for a function that is
 * set dynamically (at run time) and called from the filter. That
 * function called by the filter can also be a python callback function.
 *******************************************************************/
typedef int (*subfilter_type)(const char*);

// static or extern so it can be set outside the filter function
// at run time, within C/C++ or passed as a callback from python.
static subfilter_type subfilter;

// must be set to false if there is no
static bool callsubfilter=true;

void filter_function(string const& value) {
	int i = value.length();

	if(value[i-1] == '\n')
		cout << "Filter b(" << value.substr(0,i-1) << ")\n";
	else
		cout << "Filter b(" << value << ")\n";
    if(callsubfilter)
    	subfilter(value.c_str());
}

int subfilter_in_C(const char* some_text){
	string subfiltered = string("Subfilter b ") + string(some_text);
	cout << "     " << subfiltered;
	return 1;
}

int runwithfilter(subfilter_type subfilterfunc){
	subfilter = subfilterfunc;
	callsubfilter = true;
	return 1;
}
//------------- Subsection of Filter b, simple demo --------------//

//static shared_ptr<spdlog::logger> logbb;

int basic_filter_b_demo() {
    static ofunctionstream out(&filter_function);
    printf("%s", "printf not affected\n");
    cout<< "cout not affected"<<endl;
	//Two examples from stackoverflow that run through filter
    out << "hello" << ',' << " world: " << 42 << "\n";
    out << nounitbuf << "not" << " as " << "many" << " calls\n" << std::flush;
    // unitbuf so automatic flushing resumes
    out<<unitbuf;
    //Example of filter in logging. Note spdlog examples use auto in initialization, as in
    // auto ostream_sink = ... and auto log = ....
    shared_ptr<spdlog::sinks::ostream_sink<mutex> >  ostream_sink = make_shared<spdlog::sinks::ostream_sink_mt> (out);
    shared_ptr<spdlog::logger> logb = make_shared<spdlog::logger>("my_logger", ostream_sink);
    logb->info("Logging through Filter b");
    // see also https://github.com/gabime/spdlog/wiki/4.-Sinks for sink info
    return 1;
}

/*
#include <Python.h>

void run_python(const char* code){
  Py_Initialize();
  PyRun_SimpleString(code);
  Py_Finalize();
}
*/

int full_filter_b_demo(){

/* 1) use static variables which are available outside of function
 * 2) set log with filter to default which macro SPDLOG_INFO will use,
 *	  the macros SPDLOG_DEBUG, SPDLOG_INFO, etc necessary to output
 *	  file name, line number, function name, in the log output.
 */

	//callpythonlogger=callfunc;
	spdlog::info("more complex demo");

	printf("printf unaffected by filters\n");

	//unless static,logging would crash outside this function
    static ofunctionstream myout(&filter_function);
    static auto myostream_sink = make_shared<spdlog::sinks::ostream_sink_mt> (myout);
    staticlogptr = make_shared<spdlog::logger>("my_logger", myostream_sink);
    //set format for all logs, see https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] |%!| %v |%@|");
    staticlogptr->info("log output missing |function| |filename line number|");
    //set a simpler format for just this log for dev
    staticlogptr->set_pattern("%H:%M:%S | %! | %v | %@");
    staticlogptr->info("shorter pattern, still missing function, file and line number");
    SPDLOG_INFO("Use macro SPDLOG_INFO, missing info appears, but no filter");
	spdlog::set_default_logger(staticlogptr);
	staticlogptr->set_pattern("%H:%M:%S | %! | %v | %s:%#");
	// use %s:%# for file name:line number, %@ missing ':' needed for source link
    SPDLOG_INFO("set default logger to filtered logger, and use SPDLOG_INFO, filter reappears");
    SPDLOG_INFO("Note: In tweakme.h uncomment #define SPDLOG_SHORT_LEVEL_NAMES to use I instead of SPDLOG_INFO");
    // see https://github.com/gabime/spdlog/wiki/8.-Tweaking

    //This will print the line or source_loc as nonexistent_file, line 186
    //Below shows the call that the macro SPDLOG_INFO("log message") assembles in order to  output file, line number, function, and log level information
    staticlogptr->log(spdlog::source_loc{"/project/legaltexts/file_does_not_exist.cpp", 186, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, "see source, how SPDLOG_INFO works");
    spdlog::info("non pointer log syntax works through default logger and thus filter too");

    using namespace spdlog; //can be subject to name conflicts, overlap
    info("spdlog namespace for convenience");

    return 1;
}

void fun_with_strings(){
	string s =fmt::format("fmt::format included in spdlog, to insert {}, and numbers, eg {},{},{}", "text", 1,2,3  );
	printf("%s\n", s.c_str());
	s = "From printf using &s[0] instead of s.c_str() to convert string to char* for printf.\n";
	printf("%s", &s[0]);
}

extern "C" //Note: Must have extern "C" to be called from python.
int run_demos(subfilter_type subfilterfunc){
	//Filter a, one way to redirect std::out, as noted, adapted from:
	//https://stackoverflow.com/questions/1881589/redirected-cout-stdstringstream-not-seeing-eol
	redirectStdOut();
    printf("output of printf is unaffected\n");
    cout << "Line 1 of redirected cout\n";
    cout << "Line 2 of redirected cout\n";
    static auto myostream_sink_a = make_shared<spdlog::sinks::ostream_sink_mt> (cout);
    static auto loga = make_shared<spdlog::logger>("my_logger", myostream_sink_a);
    loga->info("logging also going through filter");
	printOut();
	resetStdOut();
	cout << "After resetStdOut(), back to regular std out, no redirect to Filter a\n";
	spdlog::info("logging without Filter a");

	//turn off the subfilter for now (a second filter called inside the first)
	callsubfilter=false;
	//Filter b, simple demo
	basic_filter_b_demo();

	//Filter b, more complex demo with logging
	full_filter_b_demo();

	cout<<"in main filter still present"<<endl;

	cout<<"Plain console line, static file name:line number links to source loggingexample.cpp:263\n";
	spdlog::info("spdlog::info Filter b in log output working outside of setup function, no file:line#");
	SPDLOG_INFO("called with SPDLOG_INFO, file:line# links to source if called from python");
	//some relatively trivial shenanigans with strings
	//fmt::format is slated to be std::format in C++20 but not implemented yet, available here:
	//https://github.com/fmtlib/fmt also included here: https://github.com/gabime/spdlog
	fun_with_strings();

	// subfilter
    runwithfilter(subfilterfunc);
    staticlogptr->info("This will appear in subfiltered log output.");

	return 1;
}

int main(int argc, char**argv){
	cout<<"Ways to filter or transform output streams including std::cout"<<endl
	    <<"Logging, can be run through filter and subfilter"<<endl
	    <<"Logging can include filename:line#, links to source in Eclipse if called from python"<<endl
	    <<"Miscellaneous notes on fmt::format and strings"<<endl<<endl;
	run_demos(subfilter_in_C);
	return 1;
}
