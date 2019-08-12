//必须在源文件中添加, 不能再头文件中添加
extern "C" {
#include <openssl/applink.c>
};

#include "crypto_utils.h"


namespace  CryptoUtils
{

	std::string Base64Encode(const char *input, int length, bool with_new_line)
	{
		BIO * bmem = NULL;
		BIO * b64 = NULL;
		BUF_MEM * bptr = NULL;

		b64 = BIO_new(BIO_f_base64());
		if (!with_new_line) {
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		}
		bmem = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, bmem);
		BIO_write(b64, input, length);
		BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bptr);

		char * buff = (char *)malloc(bptr->length + 1);
		memcpy(buff, bptr->data, bptr->length);
		buff[bptr->length] = 0;
		BIO_free_all(b64);


		std::string  strRet(buff);
		free(buff); buff = NULL;
		return strRet;
	}

	std::string Base64Decode(const char * input, int length, bool with_new_line)
	{
		BIO * b64 = NULL;
		BIO * bmem = NULL;
		char * buffer = (char *)malloc(length);
		memset(buffer, 0, length);

		b64 = BIO_new(BIO_f_base64());
		if (!with_new_line) {
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		}
		bmem = BIO_new_mem_buf((void *)input, length);
		bmem = BIO_push(b64, bmem);
		int iDecodeLen = BIO_read(bmem, buffer, length);

		BIO_free_all(bmem);

		std::string strRet;
		strRet.append(buffer, iDecodeLen);

		free(buffer); buffer = NULL;
		return strRet;
	}


	//公钥加密
	int RSA_EncrptByPubkey(
		char *pszPubKeyFilePath,
		unsigned char *puszInData,
		int uInDataLen,
		unsigned char *puszOutData,
		int *puOutDataLen)
	{
		int ret = 0, isrclen = 0, ideslen = 0, i = 0, n = 0, ienclen = 0, rsalen = 0;
		if (pszPubKeyFilePath == NULL || puszInData == NULL || puszOutData == NULL)
			return -1;

		RSA *r = RSA_new();

		FILE *fp = NULL;
		fp = fopen(pszPubKeyFilePath, "rt");
		if (fp == NULL)
		{
			printf("fopen [%s] error\n", pszPubKeyFilePath);
			return -2;
		}

		if (NULL == (r = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL)))
		{
			fclose(fp);
			printf("PEM_read_RSA_PUBKEY error\n");
			return -3;
		}
		fclose(fp);
		//RSA_print_fp(stdout, r, 5);  

		   //加密数据最大长度，是秘钥长度-11,加密结果，是RSA_size的返回值。
		rsalen = RSA_size(r) - 11;
		unsigned char *out = (unsigned char *)malloc(rsalen + 12);

		n = uInDataLen / rsalen;
		for (i = 0; i <= n; i++)
		{
			ienclen = (i != n ? rsalen : uInDataLen % (rsalen));
			if (ienclen == 0)
				break;
			memset(out, 0, rsalen + 12);
			ret = RSA_public_encrypt(ienclen, puszInData + isrclen, out, r, RSA_PKCS1_PADDING);
			if (ret < 0)
			{
				printf("RSA_public_encrypt failed,ret is [%d]\n", ret);
				return -4;
			}

			isrclen += ienclen;
			memcpy(puszOutData + ideslen, out, ret);
			ideslen += ret;
		}
		puszOutData[ideslen] = 0;
		*puOutDataLen = ideslen;

		free(out);
		RSA_free(r);
		CRYPTO_cleanup_all_ex_data();

		return 0;
	}



	//私钥解密
	int RSA_DecrptByPrivkey(
		char *pszPrivKeyFilePath,
		unsigned char *puszInData,
		int uInDataLen,
		unsigned char *puszOutData,
		int *puOutDataLen)
	{
		int ret = 0, isrclen = 0, ideslen = 0, i = 0, n = 0, ienclen = 0, rsalen = 0;

		if (pszPrivKeyFilePath == NULL || puszInData == NULL || puszOutData == NULL)
			return -1;

		RSA *r = RSA_new();

		FILE *fp = NULL;
		fp = fopen(pszPrivKeyFilePath, "rt");
		if (fp == NULL)
		{
			printf("fopen [%s] error\n", pszPrivKeyFilePath);
			return -2;
		}


		if (PEM_read_RSAPrivateKey(fp, &r, 0, 0) == NULL)
		{
			fclose(fp);
			printf("PEM_read_RSAPrivateKey error\n");
			return -3;
		}
		fclose(fp);
		//RSA_print_fp(stdout, r, 5);

		rsalen = RSA_size(r);
		unsigned char *out = (unsigned char *)malloc(rsalen + 1);

		n = uInDataLen / rsalen;
		for (i = 0; i <= n; i++)
		{

			ienclen = (i != n ? rsalen : uInDataLen % (rsalen));
			if (ienclen == 0)
				break;
			memset(out, 0, rsalen + 1);
			ret = RSA_private_decrypt(ienclen, puszInData + isrclen, out, r, RSA_PKCS1_PADDING);
			if (ret < 0)
			{
				printf("RSA_private_decrypt failed,ret is [%d]\n", ret);
				return -4;
			}

			isrclen += ienclen;
			memcpy(puszOutData + ideslen, out, ret);
			ideslen += ret;
		}
		puszOutData[ideslen] = 0;
		*puOutDataLen = ideslen;

		free(out);
		RSA_free(r);
		CRYPTO_cleanup_all_ex_data();

		return 0;
	}


	//私钥加密
	int RSA_EncrptByPrivKey(
		char *pszPrivKeyFilePath,
		unsigned char *puszInData,
		int uInDataLen,
		unsigned char *puszOutData,
		int *puOutDataLen)
	{
		int ret = 0, isrclen = 0, ideslen = 0, i = 0, n = 0, ienclen = 0, rsalen = 0;
		if (pszPrivKeyFilePath == NULL || puszInData == NULL || puszOutData == NULL)
			return -1;

		RSA *r = RSA_new();

		FILE *fp = NULL;
		fp = fopen(pszPrivKeyFilePath, "rt");
		if (fp == NULL)
		{
			RSA_free(r);
			printf("fopen [%s] error\n", pszPrivKeyFilePath);
			return -2;
		}

		if (PEM_read_RSAPrivateKey(fp, &r, 0, 0) == NULL)
		{
			RSA_free(r);
			fclose(fp);
			printf("PEM_read_RSAPrivateKey error\n");
			return -3;
		}
		fclose(fp);


		//RSA_print_fp(stdout, r, 5);
		//加密数据最大长度，是秘钥长度-11,加密结果，是RSA_size的返回值。
		rsalen = RSA_size(r) - 11;
		unsigned char *out = (unsigned char *)malloc(rsalen + 12);

		n = uInDataLen / rsalen;
		for (i = 0; i <= n; i++)
		{
			ienclen = (i != n ? rsalen : uInDataLen % (rsalen));
			if (ienclen == 0)
				break;
			memset(out, 0, rsalen + 12);
			ret = RSA_private_encrypt(ienclen, puszInData + isrclen, out, r, RSA_PKCS1_PADDING);
			if (ret < 0)
			{
				free(out);
				RSA_free(r);
				printf("RSA_private_encrypt failed,ret is [%d]\n", ret);
				return -4;
			}

			isrclen += ienclen;
			memcpy(puszOutData + ideslen, out, ret);
			ideslen += ret;
		}
		puszOutData[ideslen] = 0;
		*puOutDataLen = ideslen;
		free(out);
		RSA_free(r);
		CRYPTO_cleanup_all_ex_data();

		return 0;
	}



	//公钥解密
	int RSA_DecryptByPubkey(
		char *pszPubKeyFilePath,
		unsigned char *puszInData,
		int uInDataLen,
		unsigned char *puszOutData,
		int *puOutLen)
	{
		int ret = 0, isrclen = 0, ideslen = 0, i = 0, n = 0, ienclen = 0, rsalen = 0;

		if (pszPubKeyFilePath == NULL || puszInData == NULL || puszOutData == NULL)
			return -1;

		RSA *r = RSA_new();

		FILE *fp = NULL;
		fp = fopen(pszPubKeyFilePath, "rt");
		if (fp == NULL)
		{
			RSA_free(r);
			printf("fopen [%s] error\n", pszPubKeyFilePath);
			return -2;
		}
		if (PEM_read_RSA_PUBKEY(fp, &r, 0, 0) == NULL)
		{
			RSA_free(r);
			fclose(fp);
			printf("PEM_read_RSA_PUBKEY error\n");
			return -3;
		}
		fclose(fp);
		//RSA_print_fp(stdout, r, 5);
		rsalen = RSA_size(r);
		unsigned char *out = (unsigned char *)malloc(rsalen + 1);

		n = uInDataLen / rsalen;
		for (i = 0; i <= n; i++)
		{

			ienclen = (i != n ? rsalen : uInDataLen % (rsalen));
			if (ienclen == 0)
				break;
			memset(out, 0, rsalen + 1);
			ret = RSA_public_decrypt(ienclen, puszInData + isrclen, out, r, RSA_PKCS1_PADDING);
			if (ret < 0)
			{
				free(out);
				RSA_free(r);
				printf("RSA_public_decrypt failed,ret is [%d]\n", ret);
				return -4;
			}

			isrclen += ienclen;
			memcpy(puszOutData + ideslen, out, ret);
			ideslen += ret;
		}
		puszOutData[ideslen] = 0;
		*puOutLen = ideslen;

		free(out);
		RSA_free(r);
		CRYPTO_cleanup_all_ex_data();

		return 0;
	}




	//私钥签名EVP
	int SHA256WithRSA_Sign(
		char *pszPrivKeyFilePath,
		unsigned char *puszInData,
		unsigned int uInDataLen,
		unsigned char *puszOutSig,
		unsigned int *puOutSigLen)
	{
		if (NULL == pszPrivKeyFilePath || NULL == puszInData || NULL == puszOutSig)
		{
			printf("SHA256WithRSA_Verify args error!\n");
			return -1;
		}

		//RSA结构体变量
		RSA *rsa_pri_key = RSA_new();

		FILE *fp = NULL;
		fp = fopen(pszPrivKeyFilePath, "rt");
		if (fp == NULL)
		{
			printf("fopen [rsa_private_key] error\n");
			return -1;
		}
		if (!PEM_read_RSAPrivateKey(fp, &rsa_pri_key, 0, 0))
		{
			fclose(fp);
			printf("PEM_read_RSAPrivateKey error\n");
			return -1;
		}
		fclose(fp);

		//新建一个EVP_PKEY变量
		EVP_PKEY *evpKey = EVP_PKEY_new();

		if (evpKey == NULL)
		{
			printf("EVP_PKEY_new err\n");
			RSA_free(rsa_pri_key);
			return -1;
		}
		if (EVP_PKEY_set1_RSA(evpKey, rsa_pri_key) != 1)   //保存RSA结构体到EVP_PKEY结构体  
		{
			printf("EVP_PKEY_set1_RSA err\n");
			RSA_free(rsa_pri_key);
			EVP_PKEY_free(evpKey);
			return -1;
		}
		//摘要算法上下文变量  
		EVP_MD_CTX mdctx;
		//以下是计算签名代码   
		EVP_MD_CTX_init(&mdctx);
		//初始化摘要上下文   
		//if (!EVP_SignInit_ex(&mdctx, EVP_sha1(), NULL)) //签名初始化，设置摘要算法，本例为sha1  
		if (!EVP_SignInit_ex(&mdctx, EVP_sha256(), NULL)) //签名初始化，设置摘要算法，本例为sha1  
		{
			printf("err\n");
			EVP_PKEY_free(evpKey);
			RSA_free(rsa_pri_key);
			return -1;
		}
		if (!EVP_SignUpdate(&mdctx, puszInData, uInDataLen)) //计算签名（摘要）Update
		{
			printf("err\n");
			EVP_PKEY_free(evpKey);
			RSA_free(rsa_pri_key);
			return -1;
		}
		if (!EVP_SignFinal(&mdctx, puszOutSig, puOutSigLen, evpKey)) //签名输出  
		{
			printf("err\n");
			EVP_PKEY_free(evpKey);
			RSA_free(rsa_pri_key);
			return -1;
		}

		//释放内存
		EVP_PKEY_free(evpKey);
		RSA_free(rsa_pri_key);
		EVP_MD_CTX_cleanup(&mdctx);

		return 0;
	}


	//公钥签名EVP验证
	int SHA256WithRSA_Verify(
		char *pszPubKeyFilePath,
		unsigned char *puszInData,
		int uInDataLen,
		unsigned char *puszOutSig,
		unsigned int puOutSigLen)
	{

		if (NULL == pszPubKeyFilePath || NULL == puszInData || NULL == puszOutSig)
		{
			printf("SHA256WithRSA_Verify args error!\n");
			return -1;
		}

		//摘要算法上下文变量 
		EVP_MD_CTX mdctx;

		RSA *rsa_pub_key = RSA_new();

		//rsa_public_key测试
		FILE *fp = NULL;
		fp = fopen(pszPubKeyFilePath, "rt");
		if (fp == NULL)
		{
			printf("fopen [rsa_private_key] error\n");
			return -2;
		}
		if (!PEM_read_RSA_PUBKEY(fp, &rsa_pub_key, NULL, NULL))
		{
			fclose(fp);
			printf("PEM_read_RSAPrivateKey error\n");
			return -3;
		}
		fclose(fp);

		EVP_PKEY *pubKey = EVP_PKEY_new();
		//新建一个EVP_PKEY变量
		if (pubKey == NULL)
		{
			printf("EVP_PKEY_new err\n");
			RSA_free(rsa_pub_key);
			return -4;
		}
		if (EVP_PKEY_set1_RSA(pubKey, rsa_pub_key) != 1)   //保存RSA结构体到EVP_PKEY结构体  
		{
			printf("EVP_PKEY_set1_RSA err\n");
			RSA_free(rsa_pub_key);
			EVP_PKEY_free(pubKey);
			return -5;
		}

		// printf("\n正在验证签名...\n");
		 //以下是验证签名代码   
		EVP_MD_CTX_init(&mdctx);
		//初始化摘要上下文   
		//if (!EVP_VerifyInit_ex(&mdctx, EVP_sha1(), NULL))  //验证初始化，设置摘要算法，一定要和签名一致。  
		if (!EVP_VerifyInit_ex(&mdctx, EVP_sha256(), NULL))  //验证初始化，设置摘要算法，一定要和签名一致。  
		{
			printf("EVP_VerifyInit_ex err\n");
			EVP_PKEY_free(pubKey);
			RSA_free(rsa_pub_key);
			return -6;
		}
		if (!EVP_VerifyUpdate(&mdctx, puszInData, uInDataLen))
			//验证签名（摘要）Update
		{
			printf("err\n");
			EVP_PKEY_free(pubKey);
			RSA_free(rsa_pub_key);
			return -7;
		}
		if (!EVP_VerifyFinal(&mdctx, puszOutSig, puOutSigLen, pubKey))//验证签名
		{
			printf("verify err\n");
			EVP_PKEY_free(pubKey);
			RSA_free(rsa_pub_key);
			return -8;
		}
		//else
		//{
			//	printf("验证签名正确.\n");
		//}

		EVP_PKEY_free(pubKey);
		RSA_free(rsa_pub_key);
		EVP_MD_CTX_cleanup(&mdctx);

		return 0;
	}



	/*
	https://www.cnblogs.com/cocoajin/p/6121706.html

	EVP_CipherInit_ex 初始化加密使用的key, iv，算法模式，最后 一个参数，1表示加密，0表示解密

	EVP_CipherUpdate 加密解密处理

	EVP_CipherFinal 获取结果
	*/
	//加密
	int AES_128_CBC_Encrypt(
		unsigned char *puszInData,
		int nInDataLen,
		unsigned char *puszKey,
		unsigned char *puszIV,
		unsigned char *puszOutData,
		unsigned int *pOutDataLen)
	{
		EVP_CIPHER_CTX *ctx;

		int nTmpLen;

		int nOutDataLen;
		ctx = EVP_CIPHER_CTX_new();
		EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, puszKey, puszIV);
		EVP_EncryptUpdate(ctx, puszOutData, &nTmpLen, puszInData, nInDataLen);
		nOutDataLen = nTmpLen;
		EVP_EncryptFinal_ex(ctx, puszOutData + nTmpLen, &nTmpLen);
		nOutDataLen += nTmpLen;
		EVP_CIPHER_CTX_free(ctx);

		//return nOutDataLen;
		*pOutDataLen = nOutDataLen;
		return nOutDataLen;
	}

	//解密
	int AES_128_CBC_Decrypt(
		unsigned char *puszInData,
		int nInDataLen,
		unsigned char *puszKey,
		unsigned char *puszIV,
		unsigned char *puszOutData,
		unsigned int *pOutDataLen
	)
	{
		EVP_CIPHER_CTX *ctx;

		int nTmpLen;
		unsigned int nOutDataLen;

		ctx = EVP_CIPHER_CTX_new();

		EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, puszKey, puszIV);
		EVP_DecryptUpdate(ctx, puszOutData, &nTmpLen, puszInData, nInDataLen);
		nOutDataLen = nTmpLen;

		EVP_DecryptFinal_ex(ctx, puszOutData + nTmpLen, &nTmpLen);
		nOutDataLen += nTmpLen;

		EVP_CIPHER_CTX_free(ctx);

		*pOutDataLen = nOutDataLen;
		return nOutDataLen;
	}

}