# Eigen2Mat
This is a tiny header only Eigen to Matlab (and vice versa) interface based on the MatlabEngine and MatlabDataArray libraries for easy and fast data exchange of the commonly used (dynamic) Eigen data types Vector and Matrix as well as ```std::vector<ScalarType>```.
This is particularly useful for rapid prototyping and validation of C++ algorithms against Matlab implementations, plotting & data visualization as well as wrapping and controlling Matlab code from within your C++ app.
Besides abstracting away the tedious data conversions from and to the shared engine, this interface also exposes some convenience methods such as evaluations, plotting and saving (all) workspace variables to files.

## Usage
```cpp
Matlab::Session m; // Start a new Matlab engine session
// Matlab::Session m("MyShare"); // or use a shared one; type matlab.engine.shareEngine("MyShare") in Matlab

std::vector<double> v(1000);
std::iota(v.begin(), v.end(), 0);
m.plot(v);

const Vector r = Vector::Random(1000);
m.plot(r);
  
const Matrix R = Vector::Random(1000, 3);
m.plot(R);
  
const ScalarType x = 5;
m.set("x", x);
m.eval("Z = eval('magic(x)')");
m.eval("mesh(Z)");
```

## Requires
- Eigen3
- Matlab ≥ 2018a
