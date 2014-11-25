This is a simple model that tries to fit a quadratic equation to a set of input samples.
Of course this could be done analytically, but that's no fun, so let's use adept.

This code is intended as a demonstration of how to parallelize training of models using
adept. Paralleization is done at the level of the training examples, and managed by a
producer/consumer pattern. The main thread puts each training example in a work queue
to learn from, and the consumer threads pull examples from the queue, run the model,
and update the parameters.

The code could be more efficient, but is left as is for the sake of simplicity.
