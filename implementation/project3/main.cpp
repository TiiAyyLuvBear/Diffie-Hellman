#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
using namespace std;

// ========================== CLASS BigNum ==========================

class BigNum {
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
    static BigNum modPow(const BigNum &base, const BigNum &exp, const BigNum &mod);
};

// ========================== BigNum Implementation ==========================

BigNum::BigNum() { digits = {0}; }

BigNum::BigNum(long long val) {
    digits.clear();
    if (val == 0) {
        digits.push_back(0);
        return;
    }
    bool negative = val < 0;
    if (negative) val = -val;
    while (val > 0) {
        digits.push_back(val % 256);
        val /= 256;
    }
    if (negative) {
        digits.push_back(0xFF);
        digits.push_back(0);
    }
}

BigNum::BigNum(string hexStr) { fromReversedHex(hexStr); }

void BigNum::fromReversedHex(string hexStr) {
    string s;
    for (char c : hexStr)
        if (!isspace((unsigned char)c))
            s += c;

    reverse(s.begin(), s.end());

    if (s.empty()) { digits = {0}; return; }
    if (s.size() % 2 != 0) s = "0" + s;

    digits.clear();
    for (int i = static_cast<int>(s.size()) - 2; i >= 0; i -= 2) {
        char hi_c = s[i];
        char lo_c = s[i + 1];
        int hi = isdigit(static_cast<unsigned char>(hi_c)) ? (hi_c - '0')
                  : (toupper(static_cast<unsigned char>(hi_c)) - 'A' + 10);
        int lo = isdigit(static_cast<unsigned char>(lo_c)) ? (lo_c - '0')
                  : (toupper(static_cast<unsigned char>(lo_c)) - 'A' + 10);
        digits.push_back(((hi << 4) | lo) & 0xFF);
    }
    while (digits.size() > 1 && digits.back() == 0) digits.pop_back();
}

string BigNum::toReversedHex() const {
    static const char HEX[] = "0123456789ABCDEF";
    if (digits.empty()) return "00";

    string out;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        int b = digits[i];
        out += HEX[(b >> 4) & 0xF];
        out += HEX[b & 0xF];
    }
    if (out.empty()) return "00";
    size_t pos = out.find_first_not_of('0');
    if (pos == string::npos) return "00";
    return out.substr(pos);
}

int BigNum::cmp(const BigNum &b) const {
    if (digits.size() != b.digits.size())
        return digits.size() < b.digits.size() ? -1 : 1;
    for (int i = digits.size() - 1; i >= 0; i--)
        if (digits[i] != b.digits[i])
            return digits[i] < b.digits[i] ? -1 : 1;
    return 0;
}

bool BigNum::isZero() const { return digits.empty() || digits.size() == 1 && digits[0] == 0; }
bool BigNum::isOdd() const { return digits[0] & 1; }

BigNum BigNum::div2() const {
    BigNum r = *this;
    int carry = 0;
    for (int i = r.digits.size() - 1; i >= 0; i--) {
        int cur = r.digits[i] + carry * 256;
        r.digits[i] = cur / 2;
        carry = cur % 2;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0) r.digits.pop_back();
    return r;
}

BigNum BigNum::operator+(const BigNum &b) const {
    BigNum r;
    r.digits.assign(max(digits.size(), b.digits.size()) + 1, 0);
    int carry = 0;
    for (size_t i = 0; i < r.digits.size(); i++) {
        int sum = carry;
        if (i < digits.size()) sum += digits[i];
        if (i < b.digits.size()) sum += b.digits[i];
        r.digits[i] = sum % 256;
        carry = sum / 256;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0) r.digits.pop_back();
    return r;
}

BigNum BigNum::operator-(const BigNum &b) const {
    BigNum r;
    r.digits.assign(digits.size(), 0);
    int borrow = 0;
    for (size_t i = 0; i < digits.size(); i++) {
        int diff = digits[i] - (i < b.digits.size() ? b.digits[i] : 0) - borrow;
        if (diff < 0) {
            diff += 256;
            borrow = 1;
        } else borrow = 0;
        r.digits[i] = diff;
    }
    while (r.digits.size() > 1 && r.digits.back() == 0) r.digits.pop_back();
    return r;
}

BigNum BigNum::operator*(const BigNum &b) const {
    BigNum r;
    r.digits.assign(digits.size() + b.digits.size(), 0);
    for (size_t i = 0; i < digits.size(); i++) {
        int carry = 0;
        for (size_t j = 0; j < b.digits.size() || carry; j++) {
            long long cur = r.digits[i + j] +
                (long long)digits[i] * (j < b.digits.size() ? b.digits[j] : 0) + carry;
            r.digits[i + j] = cur % 256;
            carry = cur / 256;
        }
    }
    while (r.digits.size() > 1 && r.digits.back() == 0) r.digits.pop_back();
    return r;
}

BigNum BigNum::operator%(const BigNum &m) const {
    if (m.isZero()) return BigNum(0);
    if (this->cmp(m) < 0) return *this;

    BigNum dividend = *this;
    BigNum divisor = m;
    BigNum current(0);

    for (int i = dividend.digits.size() - 1; i >= 0; i--) {
        current = current * BigNum(256);
        current = current + BigNum(dividend.digits[i]);
        int x = 0, left = 0, right = 255;
        while (left <= right) {
            int mid = (left + right) / 2;
            BigNum t = divisor * BigNum(mid);
            if (t.cmp(current) <= 0) {
                x = mid;
                left = mid + 1;
            } else right = mid - 1;
        }
        current = current - divisor * BigNum(x);
    }
    return current;
}

BigNum BigNum::operator/(const BigNum &b) const {
    if (b.isZero()) throw runtime_error("Division by zero");

    BigNum dividend = *this;
    BigNum divisor = b;
    BigNum quotient(0);
    BigNum current(0);

    for (int i = static_cast<int>(dividend.digits.size()) - 1; i >= 0; i--) {
        current = current * BigNum(256) + BigNum(dividend.digits[i]);
        int x = 0, left = 0, right = 255;
        while (left <= right) {
            int mid = (left + right) / 2;
            BigNum t = divisor * BigNum(mid);
            if (t.cmp(current) <= 0) {
                x = mid;
                left = mid + 1;
            } else {
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

BigNum BigNum::gcd(const BigNum &a, const BigNum &b) {
    if (b.isZero()) return a;
    return gcd(b, a % b);
}

BigNum BigNum::modInverse(const BigNum &a, const BigNum &m) {
    if (m.isZero()) return BigNum(0);

    BigNum aa = a;
    BigNum mm = m;
    BigNum m0 = m;

    if (BigNum::gcd(aa, mm).cmp(BigNum(1)) != 0) {
        return BigNum(0);
    }

    BigNum x0(1), x1(0);

    while (!mm.isZero()) {
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

BigNum BigNum::modPow(const BigNum &base, const BigNum &exp, const BigNum &mod) {
    if (mod.cmp(BigNum(1)) == 0) return BigNum(0);
    
    BigNum result(1);
    BigNum b = base % mod;
    BigNum e = exp;
    
    while (!e.isZero()) {
        if (e.isOdd()) {
            result = (result * b) % mod;
        }
        e = e.div2();
        b = (b * b) % mod;
    }
    return result;
}

// ========================== CLASS ElGamalCrypto ==========================

class ElGamalCrypto {
private:
    BigNum p;         // p: prime
    BigNum g;         // g: generator of p
    BigNum x;         // x: secret key (p, g, x) = d
    BigNum c1, c2;    // Ciphertext components
    BigNum h;         // h = g^x mod p (public key)
    BigNum m;         // m: plaintext

public:
    bool readInput(const string &filename);
    void computePublicKey();
    void decrypt();
    void writeOutput(const string &filename);
    
    BigNum getP() const { return p; }
    BigNum getG() const { return g; }
    BigNum getX() const { return x; }
    BigNum getH() const { return h; }
    BigNum getC1() const { return c1; }
    BigNum getC2() const { return c2; }
    BigNum getM() const { return m; }
};

// ========================== ElGamalCrypto Implementation ==========================

// Read input: p, g, x, c1, c2 (5 lines)
bool ElGamalCrypto::readInput(const string &filename) {
    ifstream fi(filename);
    if (!fi.is_open()) return false;
    
    string sp, sg, sx, sc1, sc2;
    fi >> sp >> sg >> sx >> sc1 >> sc2;
    fi.close();

    p = BigNum(sp);
    g = BigNum(sg);
    x = BigNum(sx);
    c1 = BigNum(sc1);
    c2 = BigNum(sc2);
    
    return true;
}

// Compute public key: h = g^x mod p
void ElGamalCrypto::computePublicKey() {
    h = BigNum::modPow(g, x, p);
}

// Decrypt: m = c2 * (c1^x)^(-1) mod p
void ElGamalCrypto::decrypt() {
    BigNum c1x = BigNum::modPow(c1, x, p);
    BigNum c1xInv = BigNum::modInverse(c1x, p);
    m = (c2 * c1xInv) % p;
}

// Write output: h and m (2 lines)
void ElGamalCrypto::writeOutput(const string &filename) {
    ofstream fo(filename);
    if (!fo.is_open()) return;
    
    // Line 1: h = g^x mod p (public key)
    string hHex = h.toReversedHex();
    reverse(hHex.begin(), hHex.end());
    fo << hHex << "\n";
    
    // Line 2: m (plaintext)
    string mHex = m.toReversedHex();
    reverse(mHex.begin(), mHex.end());
    fo << mHex;
    
    fo.close();
}

// ========================== MAIN FUNCTION ==========================

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " input.txt output.txt\n";
        return 0;
    }

    ElGamalCrypto elgamal;
    
    if (!elgamal.readInput(argv[1])) {
        cout << "Cannot open input file\n";
        return 0;
    }
    
    // Compute public key h = g^x mod p
    elgamal.computePublicKey();
    
    // Decrypt message m from ciphertext (c1, c2)
    elgamal.decrypt();
    
    // Write output: h and m
    elgamal.writeOutput(argv[2]);

    cout << "=== ElGamal Decryption Debug Info ===\n";
    cout << "p  = " << elgamal.getP().toReversedHex() << endl;
    cout << "g  = " << elgamal.getG().toReversedHex() << endl;
    cout << "x  = " << elgamal.getX().toReversedHex() << endl;
    cout << "c1 = " << elgamal.getC1().toReversedHex() << endl;
    cout << "c2 = " << elgamal.getC2().toReversedHex() << endl;
    cout << "h  = " << elgamal.getH().toReversedHex() << endl;
    cout << "m  = " << elgamal.getM().toReversedHex() << endl;
    cout << "Processing completed successfully.\n";

    return 0;
}
