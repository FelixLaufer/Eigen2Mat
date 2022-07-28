// MIT License, Copyright (c) 2022 Felix Laufer

#include <Eigen2Mat/EigenTypes.h>
#include <Eigen2Mat/MatLab.h>

#include <iostream>
#include <numeric>

int main(int argc, char* argv[])
{
  Matlab::Session m; // Start a new Matlab engine session
  //Matlab::Session m("Shared"); // or use a shared one; type matlab.engine.shareEngine("Shared")

  std::vector<double> v(1000);
  std::iota(v.begin(), v.end(), 0);
  m.plot(v);

  const Vector rnd = Vector::Random(1000);
  m.plot(rnd);

  const ScalarType x = 5;
  m.set("x", x);
  m.eval("Z = eval('magic(x)')");
  m.eval("mesh(Z)");

  return 0;
}
