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

#include "SecBlock.h"
using CryptoPP::SecByteBlock;




#include <string>
#include <iostream>

//#include <Eigen/Dense>

#pragma comment(lib,"cryptlib.lib")

// 封装了一些签名验证的底层细节
namespace Signature{

	struct PublicKeyString{
		std::string Modulus_n;
		std::string PublicExponent_e;
	};
	struct PrivateKeyString{
		std::string Modulus_n;
		std::string PublicExponent_e;
		std::string PrivateExponent_d;
	};

	// 由明文消息，根据私钥生成签名
	inline std::string FromMessage(std::string message, RSA::PrivateKey privateKey){
		
		RSASS<PSS, SHA1>::Signer signer(privateKey);
		std::string signature;
		AutoSeededRandomPool rng;//????不确定这样是否可以,为什么前面需要随机种子？
		rng.Reseed();
		StringSource(message, true,
			new SignerFilter(rng, signer,
			new StringSink(signature)
			) // SignerFilter
			); // StringSource
		return signature;
	}

	// 由消息、前面，验证是否为该公钥人所持有
	inline bool Verify(std::string message, std::string signature, RSA::PublicKey publicKey){
		try{
			// Verify and Recover
			RSASS<PSS, SHA1>::Verifier verifier(publicKey);

			StringSource(message + signature, true,
				new SignatureVerificationFilter(
				verifier, NULL,
				SignatureVerificationFilter::THROW_EXCEPTION
				) // SignatureVerificationFilter
				); // StringSource

			std::cout << "身份验证通过" << std::endl;

		} // try

		catch (CryptoPP::Exception& e) {
			std::cerr << "身份验证失败" << std::endl;
			return false;
		}
		return true;
	}
};


namespace Lagrange{

	template<typename T>inline void log_msg(std::string msg, T obj){
		std::cout << msg << ":" << endl;
		std::cout << obj << endl << endl;
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

	inline std::vector<SecretPart> SecretDivide(Math::BigInt secret, int nAll, int nEnough, int bitCount = 256){
		using Math::BigInt;
		std::vector<BigInt> c(nEnough);
		c[0] = secret;
		AutoSeededRandomPool rng;
		for (int i = 1; i < nEnough; ++i){
			rng.Reseed();
			c[i].Randomize(rng, bitCount);
		}

		auto f = [&](BigInt x)->BigInt{
			BigInt sum(0l);
			for (int i = 0; i < nEnough; ++i){
				sum += c[i] * Math::bigint_pow(x, i);
			}
			return sum;
		};

		std::vector<SecretPart> keyArray(nAll);

		for (int i = 0; i < nAll; ++i){
			//int x = i + 1;
			BigInt x;
			rng.Reseed();
			x.Randomize(rng, BigInt(1), BigInt(99999));

			keyArray[i].x = x;
			keyArray[i].fx = f(x);
		}

		return keyArray;
	}
	
	inline Math::BigInt SecretReconstruct(std::vector<SecretPart> secret_part, int nEnough){
		using Math::BigInt;
		if (secret_part.size() < nEnough){
			std::cout << "人数不够" << std::endl;
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
		log_msg("自变量矩阵", A);

		auto det = A.det();
		log_msg("|A|", det);

		Math::BigIntMat A_star = A.adjoint();
		//log_msg("伴随矩阵", A_star);
		log_msg("AA*", A*A_star);


		auto m0 = A_star.row(0);	// 伴随矩阵的第一行
		//for (int i = 0; i < nEnough; ++i)
		//	m[i] = A_star(0, i);
		
		BigInt MxF(0l);//m*f
		for (int i = 0; i < nEnough; ++i)
			MxF += (m0[i] * f[i]);
		
		return MxF / BigInt(det);	// 保证了整数除法
	}

}