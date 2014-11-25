#include <iostream>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
// This weird include-esque thing allows our serialization
// methods to actually be injected into the middle of the
// definitions of the Eigen Matrix class.
// This must go before the inclusion of any Eigen headers.
#define EIGEN_DENSEBASE_PLUGIN "EigenDenseBaseAddons.h"
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


// To get boost to correclty serialization adoubles, we have to define
// this little serialization function. This function makes boost simply
// serialize the double part of the adouble. Any gradient information is
// not preserved.
namespace boost {
  namespace serialization {
    template<class Archive>
    inline void serialize(Archive& ar, adouble& a, const unsigned int file_version) {
      double x = a.value();
      ar & x;
      a = x;
    }
  }
}

adept::Stack stack;
int main() {
  // Create a matrix of adoubles, save it,
  // load it, then check that we get back
  // the same thing we put in.
  AMatrix x = AMatrix::Random(3, 7);
  {
    ofstream ofs("x.mat");
    boost::archive::text_oarchive oa(ofs);
    oa & x;
  }
  AMatrix y;
  {
    ifstream ifs("x.mat");
    boost::archive::text_iarchive ia(ifs);
    ia & y;
  }
  cerr << x.rows() << " " << y.rows() << endl;
  cerr << x.cols() << " " << y.cols() << endl;
  assert (x.rows() == y.rows());
  assert (x.cols() == y.cols());
  for (unsigned int i = 0; i < x.rows(); ++i) {
    for (unsigned int j = 0; j < x.cols(); ++j) { 
      assert (abs(x(i, j) - y(i, j)) < 1.0e-8);
    }
  }

  // Here's an example of saving an std::vector of Eigen
  // matrix types. This also works without issue.
  vector<AColumnVector> v(3);
  for (int i = 0; i < 3; ++i) {
    v[i] = AColumnVector::Random(4);
  }
  {
    ofstream ofs("vectors.mat");
    boost::archive::text_oarchive oa(ofs);
    oa & v;
  }
  vector<AColumnVector> w;
  {
    ifstream ifs("vectors.mat");
    boost::archive::text_iarchive ia(ifs);
    ia & w;
  }
  assert (v.size() == w.size());
  for (unsigned i = 0; i < v.size(); ++i) {
    assert (v[i].size() == w[i].size());
    for (unsigned j = 0; j < v[i].size(); ++j) {
      assert (abs(v[i](j) - w[i](j)) < 1.0e-8);
    }
  }
  cout << "All matrices saved and loaded successfully." << endl;

  /* Note: If you save a matrix of adoubles, you *must* load it back as
           a matrix of adoubles. If you try to load it back as a matrix
           of normal doubles, you will get broken results. The opposite
           is also true.
  */

  return 0;
}
