#include <iostream>
#include <Eigen/Dense>
#include "adept.h"

using namespace std;
using adept::adouble;

// Some helpful typedefs
typedef Eigen::Matrix<adept::adouble, Eigen::Dynamic, 1> AColumnVector;
typedef Eigen::Matrix<adept::adouble, 1, Eigen::Dynamic> ARowVector;
typedef Eigen::Matrix<adept::adouble, Eigen::Dynamic, Eigen::Dynamic> AMatrix;

typedef Eigen::Matrix<double, Eigen::Dynamic, 1> ColumnVector;
typedef Eigen::Matrix<double, 1, Eigen::Dynamic> RowVector;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Matrix;

// We have to each Eigen how to cast a double into
// a normal double. This is simple -- just use the
// .value() method!
namespace Eigen {
  namespace internal {
    template<>
    struct cast_impl<adouble, double>
    {
      static inline double run(const adouble& x)
      {
        return x.value();
      }
    };
  }
}

adept::Stack stack;
int main() {
  // Create a matrix of adoubles and a matrix of doubles
  AMatrix x = AMatrix::Random(2, 3);
  Matrix y = Matrix::Random(3, 5);

  // Multiply them together into whichever matrix type you like
  Matrix z = x.cast<double>() * y;
  AMatrix w = x * y.cast<adouble>();

  // Verify that the output is the same either way
  assert (z.rows() == w.rows());
  assert (z.cols() == w.cols());
  for (unsigned int i = 0; i < z.rows(); ++i) {
    for (unsigned int j = 0; j < z.cols(); ++j) {
      cerr << i << "\t" << j << "\t" << z(i, j) << "\t" << w(i, j) << "\n";
      assert (abs(z(i, j) - w(i, j)) < 1.0e-8);
    }
  }
  cout << "Success!" << endl;
  return 0;
}
