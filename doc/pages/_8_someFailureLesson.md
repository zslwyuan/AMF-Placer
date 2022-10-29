# Some Failure Lessons {#_8_someFailureLesson}

We were trapped by many problems during our implementation of the placer and experiments. We hope people in this area will not encounter those situations in the future so we list some lessons we learnt.

1. For the physical syhthesis, at the beginning, involving visualization can be very helpful to locate the problems and figure out what's going on with your algorithms. DO consider include this part in your development procedure.
2. All the changes/optimization in the convergence flow of physical synthesis should be progressive. Sudden changes of system parameters/option will cause many problems.
3. When you get significant improvement for a benchmark case, keep calm and test all the benchmarks before further improvement of the framework because some optimization techniques might be sensitive to the benchmark characteristics.
4. Double check when some operations are conducted for specific types of elements, for example, some cell types will have significant impact on the others.
5. Analytical model cannot improve or model everything because of the device complexity so sometimes you may give a chance to heuristic solutions.
6. When trying to adopt the existing solutions into your flow, the implict assumptions/requirements of those solutions should be clearly considered.
7. We might need a global data structure to store all the hyperparameters for better control of the entire placement flow since now, many parameters are scattered in difference class, making it hard to analyze the correlation between them for 3rd party users.