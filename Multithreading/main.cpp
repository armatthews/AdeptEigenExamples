#include <unistd.h>
#include <iostream>
#include <thread>
#include <tuple>
#include <chrono>
#include "adept.h"
#include "workqueue.h"
using namespace std;
using namespace adept;

// Each thread needs access to these two variables, so we make them global.
// This mutex is used to lock the main model when we actually go to overwrite
// it's parameters.
mutex Mutex;
// The work queue stores training examples that we want to learn from.
// Each iteration, the producer thread pushes the entire data set to this queue
// and the consumers run the model, differentiate, and update the model's
// parameters accordingly in parallel.
WorkQueue<tuple<double, double> > work_queue;

// Our simple model for this example.
// This represents a simple polynomial.
// Coeffs[n] represents the coefficient of x^n.
// Eval returns the value of f(x) where
// f(x) = a * x^0 + b * x^1 + c * x^2 ...
class Model
{
  public:
    vector<adouble> coeffs; 
    adouble eval(double x) {
      adouble v = 0.0;
      const unsigned j = coeffs.size();
      for (unsigned i = 0; i < coeffs.size(); ++i) {
        v *= x;
        v += coeffs[j - 1 - i];
      }
      return v;
    }
};

// This is the main function of the consumer threads.
// Each thread creates its own stack, then loops to infinity
// checking the work queue for work.
// When it fines work, it runs the model, computes the gradient
// with respect to the loss, and updates the model's parameters.
void run_thread(int tid, Model* model) {
  Stack stack;
  tuple<double, double> item;
  while (true) {
    // Copy the "real" model's parameters to a copy local to this thread
    Model local_model = *model;
    stack.new_recording();
    if (!work_queue.pop(item)) {
      break;
    }
    // Run the model
    double x = get<0>(item);
    double y = get<1>(item);
    adouble z = local_model.eval(x);

    // Compute the loss
    adouble loss = (z - y) * (z - y);

    // Compute gradients
    loss.set_gradient(1.0);
    stack.compute_adjoint();

    // Update the model parameters (don't forget to lock!)
    Mutex.lock(); 
    for (unsigned i = 0; i < local_model.coeffs.size(); ++i) {
      double g = local_model.coeffs[i].get_gradient();
      model->coeffs[i] += -g * 0.01;
    }
    Mutex.unlock();
  }
}

// A simple implementation of optimization using a single thread, just for comparison
void optimize_single_threaded(Stack& stack, Model& model, vector<double>& X, vector<double>& Y) {
  adouble z, d;
  adouble l, tl;
  for (unsigned i = 0; i < 100000; ++i) {
    tl = 0.0;
    for (unsigned j = 0; j < X.size(); ++j) {
      stack.new_recording();
      z = model.eval(X[j]);
      d = z - Y[j];
      l = d * d;
      l.set_gradient(1.0);
      stack.compute_adjoint();

      for (unsigned k = 0; k < model.coeffs.size(); ++k) {
        double g = model.coeffs[k].get_gradient(); 
        model.coeffs[k] -= 0.01 * g;
      }
      tl += l;
    }
    cerr << i << ": " << tl << endl;
  }
}

// Creates and starts the worker threads. Blocks until they all terminate
// (i.e. when the work queue is empty, and all items examined)
void start_worker_threads(const unsigned num_threads, Model* model) {
  thread* t = new thread[num_threads];

  for (int i = 0; i < num_threads; ++i) {
    t[i] = thread(run_thread, i, model);
  }

  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  } 
  delete[] t;
}

// An example of how to optimize our model in parallel.
// For each pass over the data, simply push all the training examples
// onto our work queue, then start up the worker threads, which will
// then consume all the training examples and update the model.
void optimize_multi_threaded(const unsigned num_threads, Model& model, vector<double>& X, vector<double>& Y) {
  double tl;
  const unsigned num_iterations = 10000;
  for (unsigned i = 0; i < num_iterations; ++i) {
    for (unsigned j = 0; j < X.size(); ++j) {
      work_queue.push(make_tuple(X[j], Y[j]));
    }
    start_worker_threads(num_threads, &model);
    cerr << "Completed " << i + 1 << "/" << num_iterations << " iterations\r"; 
  }
  cerr << "\n";
}

int main() {
  // Create some synthetic data to try to model
  // f(x) = 3x^2 - x + 1
  vector<double> X {1.0, 2.0, 3.0, 1.5};
  vector<double> Y {3.0, 11.0, 25.0, 6.25};

  Stack stack;
  Model model;
  model.coeffs.resize(3); // we want a quadratic

  //optimize_single_threaded(stack, model, X, Y);

  const unsigned num_threads = 8;
  optimize_multi_threaded(num_threads, model, X, Y);

  cout << "Final parameters: ";
  for (unsigned i = 0; i < model.coeffs.size(); ++i) {
    cout << model.coeffs[i] << " ";
  }
  cout << endl;

  return 0;
}
