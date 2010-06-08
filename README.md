Eureqa Client for Mathematica README
====================================

* * * 

[Eureqa][eq] is a software tool by [Michael Schmidt][ms] and [Hod
Lipson][hl] at Cornell for detecting hidden mathematical relationships
in quantitative data.  The aim of this project is to provide a Eureqa
Client for Mathematica, so that Mathematica users can easily connect
to a Eureqa server and search through data for relationships. (See
this [paper][paper] for details of the search method used.)


[eq]: http://ccsl.mae.cornell.edu/eureqa
[ms]: https://sites.google.com/a/cornell.edu/schmidt/
[hl]: http://www.mae.cornell.edu/lipson/
[paper]: http://ccsl.mae.cornell.edu/sites/default/files/Science09_Schmidt.pdf

Dependencies
------------

The following applications and libraries are required to build this
client.

* [Eureqa Server](http://ccsl.mae.cornell.edu/eureqa_download)
* [Mathematica 7](http://www.wolfram.com/mathematica/)
* [Boost](http://www.boost.org/) (libraries: system, serialization, date_time, and thread)
* [CMake](http://www.cmake.org/)

Building
--------

    $ cmake .
    $ make
    $ make install

The client should build on Mac OS X and Linux without issue provided
you have the dependencies outlined above.  Building the client on
Windows gave me some trouble, so for the time being I recommend only
trying it on Windows if you are willing to get your hands dirty with
some development; however, I welcome any patches to make building on
Windows easier.

Running
-------

Once the client has been built and installed, it can be accessed from
within Mathematica with the following commands.

    In[1]:= <<EureqaClient`
    In[2]:= ?EureqaSearch

Here is an example of how to perform a search within Mathematica.

    In[3]:= data = Table[{t,Sin[t]},{t,0,4 Pi, 0.1}]
    In[4]:= EureqaSearch[data, "x = f(t)", VariableLabels -> {"t", "x"}, 
            Host -> "10.211.55.3"]


Limitations
-----------

The client currently

* can only handle one connection;
* needs more documentation; 
* has no integrated plotting; and
* does not calculate different error metrics automatically.

Soliciting Contributions
------------------------

If you have Mathematica Workbench, I would be indebted to you for
creating the documentation templates.

I welcome help making the Windows build and installation work.  I
think the trouble lies somewhere in the FindMathLink CMake module.

License
-------

I am currently electing to be conservative and license this under the
[GNU General Public License](http://www.gnu.org/licenses/gpl.html);
however, I am open to considering other licenses, if anyone wants to
suggest a better license for this project.

Acknowledgments
---------------

Thanks to Michael Schmidt and Hod Lipson for making such a fine tool
available.

Thanks to Todd Gayley for [A MathLink Tutorial][mltut].

Thanks to Kashif Rasul, Jan Woetzel, Bart Janssen and Roel Jordans for
their work on the [FindMathLink][fml] CMake module.

[mltut]: http://www.edenwaith.com/development/tutorials/mathlink/ML_Tut.pdf
[fml]: http://github.com/kashif/FindMathLink

* * * 
<small>Written by Shane Celis on 2010-06-06.</small>
