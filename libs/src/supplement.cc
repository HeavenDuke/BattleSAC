#include <string>
#include <iostream>
#include <sstream>
#include <nan.h>

using namespace Nan;

#include "rsa.h"
using CryptoPP::RSA;
using CryptoPP::RSASS;
using CryptoPP::InvertibleRSAFunction;

#include "pssr.h"
using CryptoPP::PSS;

#include "sha.h"
using CryptoPP::SHA1;

#include "files.h"
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include "filters.h"
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;
using CryptoPP::StringSink;
using CryptoPP::StringSource;

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "secblock.h"
using CryptoPP::SecByteBlock;


#pragma comment(lib,"cryptlib.lib")


// 封装了秘密分解的底层细节
namespace Lagrange{

	template<typename T>inline void log_msg(std::string msg, T obj){
		std::cout << msg << ":" << std::endl;
		std::cout << obj << std::endl << std::endl;
	}

	namespace Math{
		template<typename T>
		class  IntMatrix{

		private:
			T*	mData = NULL;
			int mN = -1;
			typedef IntMatrix<T> ThisType;

		public:
			IntMatrix(int n){
				this->re_initialize(n);
			}

			IntMatrix(const IntMatrix& _other){
				this->re_initialize(_other.mN);
				for (int i = 0; i < mN*mN; ++i)
					this->mData[i] = _other.mData[i];
			}

			IntMatrix& operator =(const IntMatrix& _other){

				this->re_initialize(_other.mN);
				int n = _other._mN;
				for (int i = 0; i < n*n; ++i)
					this->mData[i] = _other.mData[i];

				return *this;
			}

			~IntMatrix(){
				this->safe_release();
			}

			inline friend std::ostream& operator<<(std::ostream& out, ThisType&	A){
				for (int i = 0; i < A.mN; ++i){
					for (int j = 0; j < A.mN; ++j)
						out << A(i, j) << ' ';
					out << std::endl;
				}
				return out;
			}

			inline void safe_release(){
				if (mData)
					delete[] mData;
				mData = NULL;
			}

			inline void re_initialize(int n){
				mN = n;
				this->safe_release();
				mData = new T[n*n];
				for (int i = 0; i < n*n; ++i)
					this->mData[i] = T(0l);
			}

			inline T& operator()(int i, int j){
				return mData[i*mN + j];
			}

			inline ThisType operator*(ThisType& _other){
				ThisType ret(mN);
				ThisType& _this = (*this);

				for (int i = 0; i < mN; ++i)
				for (int j = 0; j < mN; ++j)
				for (int k = 0; k < mN; ++k)
					ret(i, j) += _this(i, k) * _other(k, j);
				return ret;
			}

			inline ThisType reduce(int i_del, int j_del){

				ThisType ret(mN - 1);

				for (int i = 0; i < mN - 1; ++i)
				for (int j = 0; j < mN - 1; ++j){
					int i_tmp = i;
					int j_tmp = j;
					if (i >= i_del)
						i_tmp++;
					if (j >= j_del)
						j_tmp++;
					ret(i, j) = (*this)(i_tmp, j_tmp);
				}

				return ret;
			}

			inline T algebraic_cofactor(int i, int j){
				// 代数余子式
				T m = this->reduce(i, j).det();
				if ((i + j) % 2 == 1)
					return -m;
				return m;
			}

			std::vector<T> row(int row){
				std::vector<T> mi(mN);	// 伴随矩阵的第一行
				for (int j = 0; j < mN; ++j)
					mi[j] = (*this)(row, j);
				return mi;
			}

			inline T det(){
				if (mN == 1)return mData[0];

				T sum(0l);

				for (int i = 0; i < mN; ++i){
					T _coff = (*this)(0, i);
					T _M = this->reduce(0, i).det();
					if (i % 2 == 1)
						_M = _M*(-1);
					sum += (_coff*_M);
				}
				return sum;
			}

			ThisType transpose(){
				ThisType ret(mN);

				for (int i = 0; i < mN; ++i)
				for (int j = 0; j < mN; ++j)
					ret(i, j) = (*this)(j, i);

				return ret;
			}

			ThisType adjoint(){

				ThisType ret(mN);

				if (1==mN){
                    ret(0, 0) = 1;
                    return ret;
                }


				for (int i = 0; i < mN; ++i)
				for (int j = 0; j < mN; ++j)
					ret(i, j) = this->algebraic_cofactor(i, j);

				return ret.transpose();

			}
		};

		typedef CryptoPP::Integer		BigInt;
		typedef long					SmallInt;
		typedef IntMatrix<BigInt>		BigIntMat;
		typedef IntMatrix<SmallInt>		SmallIntMat;

		BigInt bigint_pow(const BigInt& x, const int y){
			BigInt ret(1l);

			for (int i = 0; i < y; ++i)
				ret *= x;

			return ret;
		};
	}

	struct SecretPart{
		Math::BigInt	x;
		Math::BigInt	fx;

		inline friend std::ostream& operator<<(std::ostream& out, SecretPart&	_this){
			out << "SecretPart " << std::endl;
			out << "\tx:" << _this.x << std::endl;
			out << "\tfx:" << _this.fx << std::endl;
			return out;
		}
	};

	// lambda表达式的替代品，计算目标多项式函数的值
	inline Math::BigInt f(Math::BigInt x, int nEnough, const std::vector<Math::BigInt>& c){
		Math::BigInt sum(0l);
		for (int i = 0; i < nEnough; ++i){
			sum += c[i] * Math::bigint_pow(x, i);
		}
		return sum;
	}

	inline std::vector<SecretPart> SecretDivide(Math::BigInt secret, int nAll, int nEnough, int bitCount = 256){
		using Math::BigInt;
		std::vector<BigInt> c(nEnough);
		c[0] = secret;
		AutoSeededRandomPool rng;
		for (int i = 1; i < nEnough; ++i){
			rng.Reseed();
			c[i].Randomize(rng, bitCount);
		}

		// lambda表达式被取消使用
		//auto f = [&](BigInt x)->BigInt{
		//	BigInt sum(0l);
		//	for (int i = 0; i < nEnough; ++i){
		//		sum += c[i] * Math::bigint_pow(x, i);
		//	}
		//	return sum;
		//};

		std::vector<SecretPart> keyArray(nAll);

		for (int i = 0; i < nAll; ++i){
			//int x = i + 1;
			BigInt x;
			rng.Reseed();
			x.Randomize(rng, BigInt(1), BigInt(99999));

			keyArray[i].x = x;
			//keyArray[i].fx = f(x);
			keyArray[i].fx = f(x, nEnough, c);
		}

		return keyArray;
	}

	inline Math::BigInt SecretReconstruct(std::vector<SecretPart> secret_part, int nEnough){
		using Math::BigInt;
		if (secret_part.size() < nEnough){
//			std::cout << "人数不够" << std::endl;
			return BigInt(-1);
		}

		std::vector<BigInt> x(nEnough);
		std::vector<BigInt> f(nEnough);

		for (int i = 0; i < nEnough; ++i){
			x[i] = secret_part[i].x;
			f[i] = secret_part[i].fx;
		}


		Math::BigIntMat A(nEnough);
		for (int i = 0; i < nEnough; ++i)
		for (int j = 0; j < nEnough; ++j)
			A(i, j) = Math::bigint_pow(x[i], j);
//		log_msg("自变量矩阵", A);

		Math::BigInt det = A.det();
//		log_msg("|A|", det);

		Math::BigIntMat A_star = A.adjoint();
//		log_msg("AA*", A*A_star);

		std::vector<Math::BigInt> m0 = A_star.row(0);	// 伴随矩阵的第一行

		BigInt MxF(0l);//m*f
		for (int i = 0; i < nEnough; ++i)
			MxF += (m0[i] * f[i]);

		return MxF / BigInt(det);	// 保证了整数除法
	}

}

namespace LagrangeJsonStrAPI{

	inline CryptoPP::Integer Str2BigInt(std::string str){
		return CryptoPP::Integer(str.c_str());
	}
	inline std::string BigInt2Str(CryptoPP::Integer bigint){
		std::stringstream ss;
		std::string str;

		ss << bigint;
		ss >> str;
		return str;
	}

	class SecretPartString{
	public:
		std::string	x;
		std::string	fx;

		SecretPartString(std::string _x, std::string _fx) {
		    x = _x;
		    fx = _fx;
		}

		SecretPartString(Lagrange::SecretPart s){
			x = BigInt2Str(s.x);
			fx = BigInt2Str(s.fx);
		}
		Lagrange::SecretPart toSecretPart(){
			Lagrange::SecretPart _ret;
			_ret.x = Str2BigInt(x);
			_ret.fx = Str2BigInt(fx);
			return _ret;
		}

		inline friend std::ostream& operator<<(std::ostream& out, SecretPartString&	_this){
			out << "SecretPart " << std::endl;
			out << "\tx:" << _this.x << std::endl;
			out << "\tfx:" << _this.fx << std::endl;
			return out;
		}
	};

	typedef std::vector <SecretPartString > Keys;
	typedef std::pair <std::string, Keys> Box_Keys;

	inline Box_Keys RandomlyGenerateBoxKeys(int nAll, int nEnough, int secret_len = 64, int bitCount = 256){

		CryptoPP::Integer x;
		AutoSeededRandomPool rng;
		rng.Reseed();
		x.Randomize(rng, secret_len);// 32位的密码
		x = x.AbsoluteValue();// 密码不能为负数

		std::string secret = BigInt2Str(x);


		std::vector<Lagrange::SecretPart> keyArray =
			Lagrange::SecretDivide(Str2BigInt(secret), nAll, nEnough, bitCount);

		std::vector<SecretPartString> keyArrayStr;
		for (int i = 0; i < keyArray.size(); ++i)
			keyArrayStr.push_back(SecretPartString(keyArray[i]));


		std::pair <
			std::string,
			std::vector < SecretPartString >> ret;

		ret.first = secret;
		ret.second = keyArrayStr;
		// 返回值的first 为 secret
		// 返回值的second 为 keyArrayStr
		return ret;
	}

	inline bool SecretReconstruct(std::vector<SecretPartString> keyArrayStr, std::string secret, int nEnough){
		std::vector<Lagrange::SecretPart> keyArray;
		for (int i = 0; i < keyArrayStr.size(); ++i)
			keyArray.push_back(keyArrayStr[i].toSecretPart());
		Lagrange::Math::BigInt reCons = Lagrange::SecretReconstruct(keyArray, nEnough);
		Lagrange::Math::BigInt _secret = Str2BigInt(secret);
//		std::cout << _secret << ' ' << reCons << std::endl;
		return reCons == _secret;
	}

	void GenerateCase(const FunctionCallbackInfo<v8::Value> &args) {
	    int keyNum = args[0]->NumberValue();
	    int maxNum = args[1]->NumberValue();
        Box_Keys box_keys = RandomlyGenerateBoxKeys(keyNum, maxNum);
        Keys keys = box_keys.second;
        v8::Local<v8::Object> obj = Nan::New<v8::Object>();
        v8::Local<v8::Array> _keys = Nan::New<v8::Array>();
        obj->Set(Nan::New("secret").ToLocalChecked(), Nan::New(box_keys.first).ToLocalChecked());
        for(int i = 0; i < keys.size(); i++) {
            SecretPartString key = keys[i];
            v8::Local<v8::Object> keyObj = Nan::New<v8::Object>();
            keyObj->Set(Nan::New("x").ToLocalChecked(), Nan::New(key.x).ToLocalChecked());
            keyObj->Set(Nan::New("fx").ToLocalChecked(), Nan::New(key.fx).ToLocalChecked());
            _keys->Set(i, keyObj);
        }
        obj->Set(Nan::New("keys").ToLocalChecked(), _keys);
        args.GetReturnValue().Set(obj);
    }

    void OpenCase(const FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::Object> params = args[0]->ToObject();
        int nOpen = params->Get(Nan::New("nOpen").ToLocalChecked())->NumberValue();
        v8::Local<v8::String> secret = params->Get(Nan::New("secret").ToLocalChecked())->ToString();
        std::string _secret = std::string(*(v8::String::Utf8Value(secret)));
        v8::Local<v8::Array> _keys = v8::Local<v8::Array>::Cast(params->Get(Nan::New("keys").ToLocalChecked()));
        Keys keys;
        for(int i = 0; i < _keys->Length(); i++) {
            v8::Local<v8::Object> key = _keys->Get(i)->ToObject();
            v8::Local<v8::String> temp = key->Get(Nan::New("x").ToLocalChecked())->ToString();
            std::string x = std::string(*(v8::String::Utf8Value(temp)));
            temp = key->Get(Nan::New("fx").ToLocalChecked())->ToString();
            std::string fx = std::string(*(v8::String::Utf8Value(temp)));
            SecretPartString sps = SecretPartString(x, fx);
            keys.push_back(sps);
        }
        v8::Local<v8::Boolean> valid = Nan::New(SecretReconstruct(keys, _secret, nOpen));
        args.GetReturnValue().Set(valid);
    }

}



void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("generateCase").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(LagrangeJsonStrAPI::GenerateCase)->GetFunction());
    exports->Set(Nan::New("openCase").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(LagrangeJsonStrAPI::OpenCase)->GetFunction());
}

NODE_MODULE(supplement, init)