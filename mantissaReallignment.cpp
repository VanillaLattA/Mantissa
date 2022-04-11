#include "MyFloat.h"

/*
 * The bitset library is used to debug this program: printing out the binary representation
 * of integers.
 */
#include <bitset>
#include <stdio.h>
#include <string.h>
#include <cmath>

MyFloat::MyFloat(){
  sign = 0;
  exponent = 0;
  mantissa = 0;
}

MyFloat::MyFloat(float f){
  unpackFloat(f);
}

MyFloat::MyFloat(const MyFloat & rhs){
	sign = rhs.sign;
	exponent = rhs.exponent;
	mantissa = rhs.mantissa;
}


ostream& operator<<(std::ostream &strm, const MyFloat &f){
	//this function is complete. No need to modify it.
	strm << f.packFloat();

	return strm;
}

MyFloat MyFloat::prependMantissa() const {
	/* Prepend a leading 1 onto the mantissa for both of the floating points
	 *
	 * a). To insert the initial 1, it requires generating a bitmask with a 1 bit at
	 *	   the start, equivalent to the prepended size of mantissa (24 bits)
	 *
	 * c). OR the number's mantissa with the bitmask
	 */

	// Create a new copy of the floating point number (so we can modify it)
	MyFloat updatedFloat(*this);

	// part a.
	int prependBitmask = 1 << 23;

	// Print the bitmask that we are using
	std::bitset<32> prependFloat(prependBitmask);

	// Print the binary representation of the current float's mantissa
	std::bitset<32> floatMantissa(this->mantissa);

	// part c.
	updatedFloat.mantissa |= prependBitmask;
	std::bitset<32> updatedMantissa(updatedFloat.mantissa);

	return updatedFloat;
}

// Retrieves a number's mantissa's X'th bit
int MyFloat::determineMantissaBit(int X) const {
	// Generate the bitmask
	int zMantissaMSBMask = 1 << X;

	// Print out the binary representation of zMantissaMSBMask
	std::bitset<32> binaryMSBMask(zMantissaMSBMask);

	// Print out the binary representation of the mantissa
	std::bitset<32> binaryMantissa(this->mantissa);

	// Cancel out all the bits except for the bit at index X
	int canceledMantissa = this->mantissa & zMantissaMSBMask;

	if (canceledMantissa == zMantissaMSBMask) {
		return 1;
	}
	return 0;
}

MyFloat MyFloat::operator+(const MyFloat& rhs) const {
	// Step 1: Prepend a leading 1 onto the mantissa for both of the floating points
	////cout << "Prepend implicit 1 onto both mantissas" << endl;
	MyFloat x = this->prependMantissa();
	MyFloat y = rhs.prependMantissa();

	// Debugging purposes
	std::bitset<32> xMantissa(x.mantissa);
	std::bitset<32> yMantissa(y.mantissa);
	////cout << "X Mantissa: " << xMantissa << endl;
	////cout << "Y Mantissa: " << yMantissa << endl;

	// Stores x + y
	MyFloat z(0.0);

	// Step 2: Equal the exponents
	// Converts the floating point number with the lower exponent power to have an equal
	// exponent power as the larger floating point number

	// Which floating point number's mantissa we will have shift to the right in order
	// to align for the mantissa addition

	int exponentDelta = 0;

	// Case 1: current exponent is lower than rhs' exponent
	if (x.exponent < y.exponent) {
		exponentDelta = y.exponent - x.exponent;
		x.mantissa >>= exponentDelta;

		// Update the lower exponent to be the greater exponent
		x.exponent = y.exponent;
	}

	// Case 2: rhs' exponent is lower than current exponent
	if (y.exponent < x.exponent) {
		exponentDelta = x.exponent - y.exponent;
		y.mantissa >>= exponentDelta;

		// Update the lower exponent to be the greater exponent
		y.exponent = x.exponent;
	}

	////cout << endl;
	////cout << "Exponents after equalization: " << endl;
	////cout << "X exponent: " << x.exponent << endl;
	////cout << "Y exponent: " << y.exponent << endl;

	////cout << "\nMantissas after equalization: " << endl;
	std::bitset<32> xMantissa2(x.mantissa);
	std::bitset<32> yMantissa2(y.mantissa);
	////cout << "X Mantissa: " << xMantissa2 << endl;
	////cout << "Y Mantissa: " << yMantissa2 << endl;


	// Convert operands from signed magnitude to 2's complement
	// A number is negative if its sign bit is 1

	// Edge Case (x + -y = 0):
	bool cancel = false;
	if (x.sign == 0 && y.sign == 1 && x.mantissa == y.mantissa && x.exponent == y.exponent) {
		cancel = true;
	}
	if (x.sign == 1 && y.sign == 0 && x.mantissa == y.mantissa && x.exponent == y.exponent) {
		cancel = true;
	}

	if (x.sign == 1) {
		x.mantissa = ~x.mantissa; // Flip X's bits
		x.mantissa++;
	}
	if (y.sign == 1) {
		y.mantissa = ~y.mantissa; // Flip y's bits
		y.mantissa++;
	}

	// Step 3: Add the mantissas, exponents
	if (!cancel) {
		z.exponent = y.exponent;
	}
	z.mantissa = x.mantissa + y.mantissa;
	std::bitset<32> zMantissa(z.mantissa);
	//////cout << endl;
	////cout << "After adding mantissas: " << endl;
	////cout << "Z Mantissa: " << zMantissa << endl;

	// Important: if the result is negative, convert the mantissa back to signed magnitude by
	// inverting the bits and adding 1.
	//
	// To do this, we check the sign bit of the mantissa (which is its first bit)
	int zSignBitBitmask = 1 << 31;
	int zSignBit = (z.mantissa & zSignBitBitmask) >> 31;

	////cout << endl;
	if (zSignBit == 1) {
		////cout << "Result is negative!" << endl;
		z.mantissa = ~z.mantissa;
		z.mantissa++;

		// Z is negative
		z.sign = 1;
	}

	// Generate the mask used to retrieve the MSB of z's mantissa
	// The maximum bit length of z is 25 bits, which means that the maximum possible
	// bit index of the mantissa's MSB is 24 (25th bit)
	int msbBitIndex = 24;
	int currBit = z.determineMantissaBit(msbBitIndex);

	// Continue to search for the MSB index if the current bit is not 1 or we haven't
	// searched through all of the mantissa's bits
	while (currBit != 1 && msbBitIndex >= 0) {
		// Retrieve the current bit
		currBit = z.determineMantissaBit(msbBitIndex);

		// Stop searching if the current bit is the MSB (is 1)
		if (currBit == 1) {
			break;
		}

		// Move onto the next MSB
		msbBitIndex--;
	}

	if (msbBitIndex >= 0) {
		// Once we have retrieved the bit index of Z's mantissa's MSB, compute how far we
		// need to shift it for the mantissa to be in the format 1.M.
		int mantissaMSBDelta = msbBitIndex - 23;

		// If mantissaMSBDelta is negative, it means that we need to shift the mantissa to the left
		// If mantissaMSBDelta is positive, it means that we need to shift the mantissa to the right
		//
		// Shifting the mantissa also means that we need to shift the exponent
		// If we shifted the mantissa to the left, we need to decrease the exponent
		// If we shifted the mantissa to the right, we need to increase the exponent

		/*
		 * Important note: Shifting Z's mantissa also results in the MSB of Z's mantissa being shifted
		 * too. If we shifted the mantissa to the left, the MSB increased, and if we shifted the mantissa
		 * to the right, the MSB decreased.
		 */

		if (mantissaMSBDelta > 0) {
			mantissaMSBDelta = abs(mantissaMSBDelta);
			z.mantissa >>= mantissaMSBDelta;
			z.exponent += mantissaMSBDelta;
			msbBitIndex -= mantissaMSBDelta;
		} else {
			mantissaMSBDelta = abs(mantissaMSBDelta);
			z.mantissa <<= mantissaMSBDelta;
			z.exponent -= mantissaMSBDelta;
			msbBitIndex += mantissaMSBDelta;
		}
	}

	// Step 4: After the Z mantissa is normalized, we need to remove the implicit 1 in the mantissa
	// (located at the bit index of Z's MSB)
	// Generate the bitmask
	if (z.mantissa > 0) {
		int removeImplicitBitmask = 1 << (msbBitIndex);

		// Apply the XOR bitmask on the mantissa and the bitmask to remove the implicit 1
		std::bitset<32> binaryImplicitBitmask(removeImplicitBitmask);
		std::bitset<32> step4Mantissa(z.mantissa);
		z.mantissa ^= removeImplicitBitmask;
	}

	if (cancel) {
		z.exponent = 0;
	}

	std::bitset<32> finalZMantissa(z.mantissa);
	std::bitset<32> finalZExponent(z.exponent);
	////cout << z.sign << endl;
	////cout << z.mantissa << endl;
	////cout << z.exponent << endl;

	if (cancel) {
		z.sign = 0;
		z.exponent = 0;
		z.mantissa = 0;
	}

	return z; //you don't have to return *this. it's just here right now so it will compile
}

// Given the target exponent value, converts the floating point number to have that exponent
// value by shifting the mantissa

MyFloat MyFloat::operator-(const MyFloat& rhs) const{
	// Step 1: Prepend a leading 1 onto the mantissa for both of the floating points
	MyFloat x = this->prependMantissa();
	MyFloat y = rhs.prependMantissa();

	// Edge Case (x - y = 0): x = y
	bool cancel = false;
	//cout << "X.sign = " << x.sign << ", Y.sign = " << y.sign << endl;
	//cout << "X.mantissa = " << x.mantissa << ", Y.mantissa = " << y.mantissa << endl;
	//cout << "X.exponent = " << x.exponent << ", Y.exponent = " << y.exponent << endl;
	if (x.sign == 0 && y.sign == 0 && x.mantissa == y.mantissa && x.exponent == y.exponent) {
		cancel = true;
	}
	if (x.sign == 1 && y.sign == 1 && x.mantissa == y.mantissa && x.exponent == y.exponent) {
		cancel = true;
	}

	// Invert y's sign
	if (y.sign == 1) {
		y.sign = 0;
	} else {
		y.sign = 1;
	}

	// Stores x + y
	MyFloat z(0.0);

	// Step 2: Equal the exponents
	// Converts the floating point number with the lower exponent power to have an equal
	// exponent power as the larger floating point number

	// Which floating point number's mantissa we will have shift to the right in order
	// to align for the mantissa addition

	int exponentDelta = 0;

	// Store the original mantissas before they are modified during the equalization phase
	int xOriginalMantissa = x.mantissa;
	int yOriginalMantissa = y.mantissa;

	// Case 1: current exponent is lower than rhs' exponent
	if (x.exponent < y.exponent) {
		exponentDelta = y.exponent - x.exponent;
		x.mantissa >>= exponentDelta;

		// Retrieve the last [exponentDelta] bits from the X's original mantissa
		int lastExponentDeltaBits = (1 << exponentDelta) - 1;
		int xMantissaLastBits = xOriginalMantissa & lastExponentDeltaBits;

		// Get the original quantity shifted away
		int removedQuantity = pow(2, xMantissaLastBits);

		// We have a CARRY that we need to add to X's mantissa!
		if (2 * xMantissaLastBits >= removedQuantity) {
			x.mantissa++;
		}

		// Update the lower exponent to be the greater exponent
		x.exponent = y.exponent;
	}

	// Case 2: rhs' exponent is lower than current exponent
	if (y.exponent < x.exponent) {
		exponentDelta = x.exponent - y.exponent;
		y.mantissa >>= exponentDelta;

		// Update the lower exponent to be the greater exponent
		y.exponent = x.exponent;
	}

	// Convert operands from signed magnitude to 2's complement
	// A number is negative if its sign bit is 1
	if (x.sign == 1) {
		x.mantissa = ~x.mantissa; // Flip X's bits
		x.mantissa++;
	}
	if (y.sign == 1) {
		y.mantissa = ~y.mantissa; // Flip y's bits
		y.mantissa++;
	}

	// Display the updated exponents of X and Y (should be equal)
	std::bitset<32> updatedXMantissa(x.mantissa);
	std::bitset<32> updatedYMantissa(y.mantissa);

	// Step 3: Add the mantissas, exponents
	z.exponent = y.exponent;
	z.mantissa = x.mantissa + y.mantissa;
	std::bitset<32> zMantissa(z.mantissa);

	// Important: if the result is negative, convert the mantissa back to signed magnitude by
	// inverting the bits and adding 1.
	//
	// To do this, we check the sign bit of the mantissa (which is its first bit)
	int zSignBitBitmask = 1 << 31;
	int zSignBit = (z.mantissa & zSignBitBitmask) >> 31;

	if (zSignBit == 1) {
		z.mantissa = ~z.mantissa;
		z.mantissa++;

		// Z is negative
		z.sign = 1;
	}

	// Generate the mask used to retrieve the MSB of z's mantissa
	// The maximum bit length of z is 25 bits, which means that the maximum possible
	// bit index of the mantissa's MSB is 24 (25th bit)
	int msbBitIndex = 24;
	int currBit = z.determineMantissaBit(msbBitIndex);

	// Continue to search for the MSB index if the current bit is not 1 or we haven't
	// searched through all of the mantissa's bits
	while (currBit != 1 && msbBitIndex >= 0) {
		// Retrieve the current bit
		currBit = z.determineMantissaBit(msbBitIndex);

		// Stop searching if the current bit is the MSB (is 1)
		if (currBit == 1) {
			break;
		}

		// Move onto the next MSB
		msbBitIndex--;
	}

	// Once we have retrieved the bit index of Z's mantissa's MSB, compute how far we
	// need to shift it for the mantissa to be in the format 1.M.
	int mantissaMSBDelta = msbBitIndex - 23;

	// If mantissaMSBDelta is negative, it means that we need to shift the mantissa to the left
	// If mantissaMSBDelta is positive, it means that we need to shift the mantissa to the right
	//
	// Shifting the mantissa also means that we need to shift the exponent
	// If we shifted the mantissa to the left, we need to decrease the exponent
	// If we shifted the mantissa to the right, we need to increase the exponent

	/*
	 * Important note: Shifting Z's mantissa also results in the MSB of Z's mantissa being shifted
	 * too. If we shifted the mantissa to the left, the MSB increased, and if we shifted the mantissa
	 * to the right, the MSB decreased.
	 */

	if (mantissaMSBDelta > 0) {
		mantissaMSBDelta = abs(mantissaMSBDelta);
		z.mantissa >>= mantissaMSBDelta;
		z.exponent += mantissaMSBDelta;
		msbBitIndex -= mantissaMSBDelta;
	} else {
		mantissaMSBDelta = abs(mantissaMSBDelta);
		z.mantissa <<= mantissaMSBDelta;
		z.exponent -= mantissaMSBDelta;
		msbBitIndex += mantissaMSBDelta;
	}

	// Step 4: After the Z mantissa is normalized, we need to remove the implicit 1 in the mantissa
	// (located at the bit index of Z's MSB)
	// Generate the bitmask
	int removeImplicitBitmask = 1 << (msbBitIndex);

	// Apply the XOR bitmask on the mantissa and the bitmask to remove the implicit 1
	std::bitset<32> binaryImplicitBitmask(removeImplicitBitmask);
	std::bitset<32> step4Mantissa(z.mantissa);
	z.mantissa ^= removeImplicitBitmask;

	std::bitset<32> finalZMantissa(z.mantissa);
	std::bitset<32> finalZExponent(z.exponent);

	if (cancel) {
		z.sign = 0;
		z.exponent = 0;
		z.mantissa = 0;
	}

	return z; //you don't have to return *this. it's just here right now so it will compile
}

bool MyFloat::operator==(const float rhs) const{
	MyFloat rhsCopy(rhs);

	if (sign == rhsCopy.sign && exponent == rhsCopy.exponent && mantissa == rhsCopy.mantissa) {
		return true;
	}

	return false; //this is just a stub so your code will compile
}


void MyFloat::unpackFloat(float f) {
	//this function must be written in inline assembly
	//extracts the fields of f into sign, exponent, and mantissa

	__asm__(
			"movl %%eax, %%ebx;"    	// Load the floating point number into the sign register
			"shr $31, %%ebx;"      	// Shift the sign bit to the end

			// Load in the bitmasks into the EDI and ESI registers
			"movl $0x7f800000, %%edi;"	// Bitmask to retrieve the exponent
			"movl $0x7fffff, %%esi;"	// Bitmask to retrieve the mantissa

			"movl %%eax, %%ecx;"		// Load the floating point number into the exponent register
			"andl %%edi, %%ecx;"		// Cancel all the bits except the mantissa bits
			"shr $23, %%ecx;"			// Shift the mantissa bits to the end

			"movl %%eax, %%edx;"		// Load the floating point number into the mantissa register
			"andl %%esi, %%edx;"		// Cancel all the bits except the mantissa bits
										// We do not need to shift the mantissa to the end (since it is
										// already at the end)

			// Load the floating pointer
			: "=b" (sign), "=c" (exponent), "=d" (mantissa)
			: "a" (f)
			: "cc", "esi", "edi"
			);
}//unpackFloat

float MyFloat::packFloat() const{

  //this function must be written in inline assembly
  //returns the floating point number represented by this
	unsigned int f;
	float f2;

	__asm__(
			// Load the sign bit into the floating point number
			"movl %%ebx, %%eax;"
			// Move the sign bit from the LSB to the MSB
			"shl $31, %%eax;"

			// Generate the bitmask for loading in the mantissa (EDI register)
			"movl %%ecx, %%edi;"
			// Right now, the mantissa bits are aligned at the end. We want to move them
			// to their proper position (bits 2-9)
			"shl $23, %%edi;"
			// Load the mantissa into the floating point number
			"orl %%edi, %%eax;"

			// Generate the bitmask for loading in the exponent (ESI register)
			"movl %%edx, %%esi;"
			// Load the exponent into the floating point number
			"orl %%esi, %%eax;"

			// Load the floating pointer
			: "= a" (f)
			: "b" (sign), "c" (exponent), "d" (mantissa)
			: "cc", "%edi", "%esi"
			);

	memcpy(&f2, &f, sizeof(f));
	return f2;
}//packFloat
//