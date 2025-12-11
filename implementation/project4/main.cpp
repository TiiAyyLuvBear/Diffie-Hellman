#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
using namespace std;

// ========================== CLASS BigNum ==========================

class BigNum
{
private:
    vector<int> digits;

public:
    BigNum();
    BigNum(long long val);
    BigNum(string hexStr);

    void fromReversedHex(string hexStr);
    string toReversedHex() const;
    int cmp(const BigNum &b) const;
    bool isZero() const;
    bool isOdd() const;
    BigNum div2() const;

    BigNum operator+(const BigNum &b) const;
    BigNum operator-(const BigNum &b) const;
    BigNum operator*(const BigNum &b) const;
    BigNum operator%(const BigNum &b) const;
    BigNum operator/(const BigNum &b) const;
    static BigNum gcd(const BigNum &a, const BigNum &b);
    static BigNum modInverse(const BigNum &a, const BigNum &m);
    static BigNum modPow(const BigNum &base, BigNum exp, const BigNum &mod);

};

// ========================== BigNum Implementation ==========================

BigNum::BigNum() { digits = {0}; }

BigNum::BigNum(long long val)
{
    digits.clear();
    if (val == 0)
    {
        digits.push_back(0);
        return;
    }
    bool negative = val < 0;
    if (negative)
        val = -val;
    while (val > 0)
    {
        digits.push_back(val % 256);
        val /= 256;
    }
    if (negative)
    {
        digits.push_back(0xFF);
        digits.push_back(0);
    }
}

BigNum::BigNum(string hexStr) { fromReversedHex(hexStr); }

void BigNum::fromReversedHex(string hexStr)
{
    string s;
    for (char c : hexStr)
        if (!isspace((unsigned char)c))
            s += c;

    reverse(s.begin(), s.end());

    if (s.empty())
    {
        digits = {0};
        return;
    }
    if (s.size() % 2 != 0)
        s = "0" + s;

    digits.clear();
    for (int i = static_cast<int>(s.size()) - 2; i >= 0; i -= 2)
    {
        char hi_c = s[i];
        char lo_c = s[i + 1];
        int hi = isdigit(static_cast<unsigned char>(hi_c)) ? (hi_c - '0')
                                                           : (toupper(static_cast<unsigned char>(hi_c)) - 'A' + 10);
        int lo = isdigit(static_cast<unsigned char>(lo_c)) ? (lo_c - '0')
                                                           : (toupper(static_cast<unsigned char>(lo_c)) - 'A' + 10);
        digits.push_back(((hi << 4) | lo) & 0xFF);
    }
    while (digits.size() > 1 && digits.back() == 0)
        digits.pop_back();
}

string BigNum::toReversedHex() const
{
    static const char HEX[] = "0123456789ABCDEF";
    if (digits.empty())
        return "00";

    string out;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i)
    {
        int b = digits[i];
        out += HEX[(b >> 4) & 0xF];
        out += HEX[b & 0xF];
    }
    if (out.empty())
        return "00";
    size_t pos = out.find_first_not_of('0');
    if (pos == string::npos)
        return "00";
    return out.substr(pos);
}

int BigNum::cmp(const BigNum &b) const
{
    if (digits.size() != b.digits.size())
        return digits.size() < b.digits.size() ? -1 : 1;
    for (int i = digits.size() - 1; i >= 0; i--)
        if (digits[i] != b.digits[i])
            return digits[i] < b.digits[i] ? -1 : 1;
    return 0;
}

bool BigNum::isZero() const { return digits.empty() || digits.size() == 1 && digits[0] == 0; }
bool BigNum::isOdd() const { return digits[0] & 1; }

BigNum BigNum::div2() const
{
    BigNum r = *this;
    int carry = 0;
    for (int i = r.digits.size() - 1; i >= 0; i--)
    {
        int cur = r.digits[i] + carry * 256;
        r.digits[i] = cur / 2;
        carry = cur % 2;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0)
        r.digits.pop_back();
    return r;
}

BigNum BigNum::operator+(const BigNum &b) const
{
    BigNum r;
    r.digits.assign(max(digits.size(), b.digits.size()) + 1, 0);
    int carry = 0;
    for (size_t i = 0; i < r.digits.size(); i++)
    {
        int sum = carry;
        if (i < digits.size())
            sum += digits[i];
        if (i < b.digits.size())
            sum += b.digits[i];
        r.digits[i] = sum % 256;
        carry = sum / 256;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0)
        r.digits.pop_back();
    return r;
}

BigNum BigNum::operator-(const BigNum &b) const
{
    BigNum r;
    r.digits.assign(digits.size(), 0);
    int borrow = 0;
    for (size_t i = 0; i < digits.size(); i++)
    {
        int diff = digits[i] - (i < b.digits.size() ? b.digits[i] : 0) - borrow;
        if (diff < 0)
        {
            diff += 256;
            borrow = 1;
        }
        else
            borrow = 0;
        r.digits[i] = diff;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0)
        r.digits.pop_back();
    return r;
}

BigNum BigNum::operator*(const BigNum &b) const
{
    BigNum r;
    r.digits.assign(digits.size() + b.digits.size(), 0);
    for (size_t i = 0; i < digits.size(); i++)
    {
        int carry = 0;
        for (size_t j = 0; j < b.digits.size() || carry; j++)
        {
            long long cur = r.digits[i + j] +
                            (long long)digits[i] * (j < b.digits.size() ? b.digits[j] : 0) + carry;
            r.digits[i + j] = cur % 256;
            carry = cur / 256;
        }
    }
    while (r.digits.size() > 1 && r.digits.back() == 0)
        r.digits.pop_back();
    return r;
}

BigNum BigNum::operator%(const BigNum &m) const
{
    if (m.isZero())
        return BigNum(0);
    if (this->cmp(m) < 0)
        return *this;

    BigNum dividend = *this;
    BigNum divisor = m;
    BigNum current(0);

    for (int i = dividend.digits.size() - 1; i >= 0; i--)
    {
        current = current * BigNum(256);
        current = current + BigNum(dividend.digits[i]);
        int x = 0, left = 0, right = 255;
        while (left <= right)
        {
            int mid = (left + right) / 2;
            BigNum t = divisor * BigNum(mid);
            if (t.cmp(current) <= 0)
            {
                x = mid;
                left = mid + 1;
            }
            else
                right = mid - 1;
        }
        current = current - divisor * BigNum(x);
    }
    return current;
}

BigNum BigNum::operator/(const BigNum &b) const
{
    if (b.isZero())
        throw runtime_error("Division by zero");

    BigNum dividend = *this;
    BigNum divisor = b;
    BigNum quotient(0);
    BigNum current(0);

    for (int i = static_cast<int>(dividend.digits.size()) - 1; i >= 0; i--)
    {
        current = current * BigNum(256) + BigNum(dividend.digits[i]);
        int x = 0, left = 0, right = 255;
        while (left <= right)
        {
            int mid = (left + right) / 2;
            BigNum t = divisor * BigNum(mid);
            if (t.cmp(current) <= 0)
            {
                x = mid;
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }
        quotient.digits.insert(quotient.digits.begin(), x);
        current = current - divisor * BigNum(x);
    }
    while (quotient.digits.size() > 1 && quotient.digits.back() == 0)
        quotient.digits.pop_back();
    return quotient;
}

BigNum BigNum::gcd(const BigNum &a, const BigNum &b)
{
    if (b.isZero())
        return a;
    return gcd(b, a % b);
}

BigNum BigNum::modInverse(const BigNum &a, const BigNum &m)
{
    if (m.isZero())
        return BigNum(0);

    BigNum aa = a;
    BigNum mm = m;
    BigNum m0 = m;

    if (BigNum::gcd(aa, mm).cmp(BigNum(1)) != 0)
    {
        return BigNum(0);
    }

    BigNum x0(1), x1(0);

    while (!mm.isZero())
    {
        BigNum q = aa / mm;

        BigNum t = mm;
        mm = aa % mm;
        aa = t;

        BigNum qx1 = (q * x1) % m0;
        BigNum newx = (x0 + m0 - qx1) % m0;

        x0 = x1;
        x1 = newx;
    }

    BigNum inv = x0 % m0;
    return inv;
}

BigNum BigNum::modPow(const BigNum &base, BigNum exp, const BigNum &mod)
{
    BigNum result(1);
    BigNum b = base % mod;
    BigNum e = exp;

    while (!e.isZero())
    {
        if (e.isOdd())
            result = (result * b) % mod;
        b = (b * b) % mod;
        e = e.div2();
    }
    return result;
}

class ElgamalVerifier
{
private:
    BigNum p, g, y;
    BigNum m, r, s;

public:
    bool readInput(const string &input_path)
    {
        ifstream inputFile(input_path);
        if (!inputFile.is_open())
        {
            cerr << "Error: Cannot open input file " << input_path << endl;
            return false;
        }

        string p_hex, g_hex, y_hex, m_hex, r_hex, s_hex;
        inputFile >> p_hex >> g_hex >> y_hex >> m_hex >> r_hex >> s_hex;
        inputFile.close();

        p = BigNum(p_hex);
        g = BigNum(g_hex);
        y = BigNum(y_hex);
        m = BigNum(m_hex);
        r = BigNum(r_hex);
        s = BigNum(s_hex);

        return true;
    }

    bool elgamalVerify() const
    {
        if (r.cmp(BigNum(0)) <= 0 || r.cmp(p) >= 0)
        {
            cout << "Invalid r value" << endl;
            return false;
        }

        if (s.cmp(BigNum(0)) <= 0 || s.cmp(p - BigNum(1)) >= 0)
        {
            cout << "Invalid s value" << endl;
            return false;
        }

        BigNum left = BigNum::modPow(g, m, p);
        BigNum hr   = BigNum::modPow(y, r, p);
        BigNum rs   = BigNum::modPow(r, s, p);
        BigNum right = (hr * rs) % p;

        return left.cmp(right) == 0;
    }

    bool writeOutput(const string &output_path, bool result)
    {
        ofstream outputFile(output_path);
        if (!outputFile.is_open())
        {
            cerr << "Error: Cannot open output file " << output_path << endl;
            return false;
        }

        outputFile << (result ? "1" : "0");
        outputFile.close();
        return true;
    }
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    ElgamalVerifier verifier;

    if (!verifier.readInput(argv[1]))
        return 1;

    bool result = verifier.elgamalVerify();

    if (!verifier.writeOutput(argv[2], result))
        return 1;

    return 0;
}

