#ifndef _MATLAB_H_
#define _MATLAB_H_

#include "EigenTypes.h" 
#include <MatlabEngine.hpp>

#include <iostream>

namespace Matlab
{
  enum WorkspaceType
  {
    Base,
    Global
  };

  namespace
  {
    matlab::data::Array toMatlab(const ScalarType& s, ScalarType*)
    {
      matlab::data::ArrayFactory arrayFac;
      return arrayFac.createScalar(s);
    }

    matlab::data::Array toMatlab(const std::vector<ScalarType>& vs, std::vector<ScalarType>*)
    {
      matlab::data::ArrayFactory arrayFac;
      return arrayFac.createArray({ static_cast<unsigned int>(vs.size()), 1 }, vs.data(), vs.data() + vs.size());
    }

    template <typename T>
    matlab::data::Array toMatlab(const T& t, T*)
    {
      matlab::data::ArrayFactory arrayFac;
      return arrayFac.createArray({ static_cast<unsigned int>(t.rows()), static_cast<unsigned int>(t.cols()) }, t.data(), t.data() + t.size());
    }

    template <typename T>
    std::vector<matlab::data::Array> toMatlab(const std::vector<T>& vt, std::vector<T>*)
    {
      std::vector<matlab::data::Array> ret;
      for (const T& t : vt)
        ret.push_back(toMatlab(t, static_cast<T*>(nullptr)));
      return ret;
    }
  }

  template <typename T>
  auto toMatlab(const T& t) -> decltype(toMatlab(t, static_cast<T*>(nullptr)))
  {
    return toMatlab(t, static_cast<T*>(nullptr));
  }

  namespace
  {
    Matrix toEigen(const matlab::data::Array& a, Matrix*)
    {
      matlab::data::ArrayDimensions dims = a.getDimensions();
      const unsigned int rows = a.getDimensions()[0];
      const unsigned int cols = a.getDimensions()[1];
      Matrix ret;
      if (dims.size() != 2 || rows == 0 || cols == 0)
        return ret;
      ret.resize(rows, cols);
      for (unsigned int i = 0; i < rows; ++i)
        for (unsigned int j = 0; j < cols; ++j)
          ret(i, j) = a[i][j];
      return ret;
    }

    Vector toEigen(const matlab::data::Array& a, Vector*)
    {
      matlab::data::ArrayDimensions dims = a.getDimensions();
      const unsigned int size = a.getNumberOfElements();
      const unsigned int rows = a.getDimensions()[0];
      const unsigned int cols = a.getDimensions()[1];
      const unsigned int minDim = std::min(rows, cols);
      const unsigned int maxDim = std::min(rows, cols);
      Vector ret;
      if (dims.size() != 2 || minDim != 1 || maxDim == 0)
        return ret;
      ret.resize(size);
      for (unsigned int i = 0; i < size; ++i)
        ret(i) = a[i];
      return ret;
    }

    ScalarType toEigen(const matlab::data::Array& a, ScalarType*)
    {
      if (a.getDimensions().size() > 2 || !(a.getDimensions()[0] == 1))
        return std::numeric_limits<ScalarType>::quiet_NaN();
      return a[0];
    }

    std::vector<ScalarType> toEigen(const matlab::data::Array& a, std::vector<ScalarType>*)
    {
      Vector v = toEigen(a, static_cast<Vector*>(nullptr));
      return std::vector<ScalarType>(v.data(), v.data() + v.size());
    }

    template <typename T>
    T toEigen(const matlab::data::Array& a, T*)
    {
      return T();
    }

    template <typename T>
    std::vector<T> toEigen(const std::vector<matlab::data::Array>& va, std::vector<T>*)
    {
      std::vector<T> ret;
      for (const matlab::data::Array& a : va)
        ret.push_back(toEigen(a, static_cast<matlab::data::Array*>(nullptr)));
      return ret;
    }
  }

  template <typename T>
  T toEigen(const matlab::data::Array& a)
  {
    return toEigen(a, static_cast<T*>(nullptr));
  }

  static std::u16string toUTF16(const std::string& s)
  {
    return matlab::execution::convertUTF8StringToUTF16String(s);
  }

  static std::string toUTF8(const std::u16string& s)
  {
    return matlab::execution::convertUTF16StringToUTF8String(s);
  }


  class Session
  {
  public:
    Session()
    {
      try
      {
        std::cout << "Starting up new Matlab session..." << std::endl;
        matlabPtr_ = matlab::engine::connectMATLAB();
      }
      catch (std::exception& e)
      {
        std::cerr << "Unable to connect to a Matlab session:" << std::endl << e.what() << std::endl;
      }
    }

    Session(const std::string& sharedSessionName)
    {
      try
      {
        std::cout << "Connecting to a shared Matlab session \"" << sharedSessionName << "\"..." << std::endl;
        matlabPtr_ = matlab::engine::connectMATLAB(toUTF16(sharedSessionName));
      }
      catch (std::exception& e)
      {
        std::cerr << "Unable to connect to a shared Matlab session \"" << sharedSessionName << "\":" << std::endl << e.what() << std::endl;
      }
    }

    ~Session()
    {
      matlab::engine::terminateEngineClient();
    }

    static std::vector<std::string> find()
    {
      std::vector<std::string> ret;
      for (const auto& s : matlab::engine::findMATLAB())
        ret.push_back(toUTF8(s));
      return ret;
    }

    matlab::data::Array get(const std::string& varName, WorkspaceType workspaceType = Base) const
    {
      try
      {
        return matlabPtr_->getVariable(toUTF16(varName), workspaceType == Base ? matlab::engine::WorkspaceType::BASE : matlab::engine::WorkspaceType::GLOBAL);
      }
      catch (std::exception& e)
      {
        std::cerr << "Unable to get Matlab variable '" + varName + "'" << std::endl << e.what() << std::endl;
        return matlab::data::Array();
      }
    }

    template <typename T>
    T get(const std::string& varName, WorkspaceType workspaceType = Base) const
    {
      return toEigen<T>(get(varName, workspaceType));
    }

    void set(const std::string& varName, const matlab::data::Array& var, WorkspaceType workspaceType = Base) const
    {
      try
      {
        matlabPtr_->setVariable(toUTF16(varName), var, workspaceType == Base ? matlab::engine::WorkspaceType::BASE : matlab::engine::WorkspaceType::GLOBAL);
      }
      catch (std::exception& e)
      {
        std::cerr << "Unble to set Matlab variable '" + varName + "'" << std::endl << e.what() << std::endl;
      }
    }

    template <typename T>
    void set(const std::string& varName, const T t, WorkspaceType workspaceType = Base) const
    {
      set(varName, toMatlab(t), workspaceType);
    }

    void eval(const std::string& statement, std::string& output, std::string& error) const
    {
      try
      {
        std::shared_ptr<std::basic_stringbuf<char16_t>> outBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        std::shared_ptr<std::basic_stringbuf<char16_t>> errBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        matlabPtr_->eval(toUTF16(statement), outBuf, errBuf);
        output = toUTF8(outBuf.get()->str());
        error = toUTF8(errBuf.get()->str());
      }
      catch (std::exception& e)
      {
        std::cerr << "Matlab syntax error." << std::endl << e.what() << std::endl;
      }
    }

    void eval(const std::string& statement, std::string& output) const
    {
      std::string error;
      eval(statement, output, error);
    }

    void eval(const std::string& statement) const
    {
      std::string output;
      std::string error;
      eval(statement, output, error);
    }

    void eval(const std::vector<std::string>& statements, std::string& output, std::string& error) const
    {
      std::string statement;
      for (const auto& s : statements)
        statement += s + "\n";
      eval(statement, output, error);
    }

    void eval(const std::vector<std::string>& statements, std::string& output) const
    {
      std::string error;
      eval(statements, output, error);
    }

    std::string eval(const std::vector<std::string>& statements) const
    {
      std::string output, error;
      eval(statements, output, error);

      return !error.empty() ? error : output;
    }

    std::vector<matlab::data::Array> feval(const std::string& function, const unsigned int numReturns, const std::vector<matlab::data::Array>& args, std::string& output, std::string& error) const
    {
      try
      {
        std::shared_ptr<std::basic_stringbuf<char16_t>> outBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        std::shared_ptr<std::basic_stringbuf<char16_t>> errBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        std::vector<matlab::data::Array> ret = matlabPtr_->feval(toUTF16(function), numReturns, args, outBuf, errBuf);
        output = toUTF8(outBuf.get()->str());
        error = toUTF8(errBuf.get()->str());
        return ret;
      }
      catch (std::exception& e)
      {
        std::cerr << "Matlab sytanx error" << std::endl << e.what() << std::endl;
        return std::vector<matlab::data::Array>();
      }
    }

    matlab::data::Array feval(const std::string& function, const std::vector<matlab::data::Array>& args, std::string& output, std::string& error) const
    {
      try
      {
        std::shared_ptr<std::basic_stringbuf<char16_t>> outBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        std::shared_ptr<std::basic_stringbuf<char16_t>> errBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        matlab::data::Array ret = matlabPtr_->feval(toUTF16(function), args, outBuf, errBuf);
        output = toUTF8(outBuf.get()->str());
        error = toUTF8(errBuf.get()->str());
        return ret;
      }
      catch (std::exception& e)
      {
        std::cerr << "Matlab syntax error" << std::endl << e.what() << std::endl;;
        return matlab::data::Array();
      }
    }

    matlab::data::Array feval(const std::string& function, const matlab::data::Array& arg, std::string& output, std::string& error) const
    {
      try
      {
        std::shared_ptr<std::basic_stringbuf<char16_t>> outBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        std::shared_ptr<std::basic_stringbuf<char16_t>> errBuf = std::make_shared<std::basic_stringbuf<char16_t>>();
        matlab::data::Array ret = matlabPtr_->feval(toUTF16(function), arg, outBuf, errBuf);
        output = toUTF8(outBuf.get()->str());
        error = toUTF8(errBuf.get()->str());
        return ret;
      }
      catch (std::exception& e)
      {
        std::cerr << "Matlab syntax error" << std::endl << e.what() << std::endl;;
        return matlab::data::Array();
      }
    }

    std::vector<matlab::data::Array> feval(const std::string& function, const unsigned int numReturns, const std::vector<matlab::data::Array>& args, std::string& output) const
    {
      std::string error;
      return feval(function, numReturns, args, output, error);
    }

    matlab::data::Array feval(const std::string& function, const std::vector<matlab::data::Array>& args, std::string& output) const
    {
      std::string error;
      return feval(function, args, output, error);
    }

    matlab::data::Array feval(const std::string& function, const matlab::data::Array& arg, std::string& output) const
    {
      std::string error;
      return feval(function, arg, output, error);
    }

    std::vector<matlab::data::Array> feval(const std::string& function, const unsigned int numReturns, const std::vector<matlab::data::Array>& args) const
    {
      std::string output, error;
      return feval(function, numReturns, args, output, error);
    }

    matlab::data::Array feval(const std::string& function, const std::vector<matlab::data::Array>& args) const
    {
      std::string output, error;
      return feval(function, args, output, error);
    }

    matlab::data::Array feval(const std::string& function, const matlab::data::Array& arg) const
    {
      std::string output, error;
      return feval(function, arg, output, error);
    }

    template <typename TRet, typename TArgs>
    std::vector<TRet> feval(const std::string& function, const unsigned int numReturns, const std::vector<TArgs>& args, std::string& output, std::string& error) const
    {
      return toEigen<TRet>(feval(function, numReturns, toMatlab(args), output, error));
    }

    template <typename TRet, typename TArgs>
    TRet feval(const std::string& function, const std::vector<TArgs>& args, std::string& output, std::string& error) const
    {
      return toEigen<TRet>(feval(function, toMatlab(args), output, error));
    }

    template <typename TRet, typename TArg>
    TRet feval(const std::string& function, const TArg& arg, std::string& output, std::string& error) const
    {
      return toEigen<TRet>(feval(function, toMatlab(arg), output, error));
    }

    template <typename TRet, typename TArgs>
    std::vector<TRet> feval(const std::string& function, const unsigned int numReturns, const std::vector<TArgs>& args, std::string& output) const
    {
      return toEigen<TRet>(feval(function, numReturns, toMatlab(args), output));
    }

    template <typename TRet, typename TArgs>
    TRet feval(const std::string& function, const std::vector<TArgs>& args, std::string& output) const
    {
      return toEigen<TRet>(feval(function, toMatlab(args), output));
    }

    template <typename TRet, typename TArg>
    TRet feval(const std::string& function, const TArg& arg, std::string& output) const
    {
      return toEigen<TRet>(feval(function, toMatlab(arg), output));
    }

    template <typename TRet, typename TArgs>
    std::vector<TRet> feval(const std::string& function, const unsigned int numReturns, const std::vector<TArgs>& args) const
    {
      return toEigen<TRet>(feval(function, numReturns, toMatlab(args)));
    }

    template <typename TRet, typename TArgs>
    TRet feval(const std::string& function, const std::vector<TArgs>& args) const
    {
      return toEigen<TRet>(feval(function, toMatlab(args)));
    }

    template <typename TRet, typename TArg>
    TRet feval(const std::string& function, const TArg& arg) const
    {
      return toEigen<TRet>(feval(function, toMatlab(arg)));
    }

    template <typename T>
    void plot(const T& t, const bool holdOn = false)
    {
      feval("plot", toMatlab(t));
      if (holdOn)
        eval("hold on");
    }

    void save(const std::string& file) const
    {
      eval("save('" + file + "')");
    }

    void save(const std::string& file, const std::vector<std::string>& variables) const
    {
      std::string varStr;
      for (const auto& variable : variables)
        varStr += ", '" + variable + "'";
      eval("save('" + file + "'" + varStr + ")");
    }

  private:
    std::unique_ptr<matlab::engine::MATLABEngine> matlabPtr_;
  };
}
#endif
