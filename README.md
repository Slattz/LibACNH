# LibACNH

LibACNH is a C++ Library For Parsing Proprietary File Formats, Algorithms & Cryptography used by Animal Crossing: New Horizons (ACNH).

Special thanks to [Ninji](https://github.com/Treeki) for their [CylindricalEarth](https://github.com/Treeki/CylindricalEarth) research.

## Disclaimer

* LibACNH is not affiliated with, endorsed or approved by Nintendo or Animal Crossing: New Horizons in any way.
* LibACNH is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. *See the GNU General Public License for more details.*

## Usage & Compilation

C++11 and above is supported for compilation, though C++20 is recommended.

## Supported File Formats, Algorithms & Cryptography

### File Formats

* [BCSV](#bcsv)
* [BFTTF](#bfttf)
* [Byaml](#byaml)
* [MSBT](#msbt)
* [SARC](#sarc)

### Algorithms

* [CRC32](#crc32)
* [MurmurHash3](#murmurhash3)
* [sead::Random](#seadrandom)

### Cryptography

* [EncryptedInt](#encryptedint)
* [SaveCrypto](#savecrypto)

## BCSV

BCSV (Binary CSV) is a proprietary file format created by Nintendo. This format is a CSV file compiled as a binary format.

The BCSV class contains functions to parse and retrieve values from a `.bcsv` file.

ACNH uses [CRC32](#crc32) to hash a majority of column names in these files.

## BFTTF

BFTTF is a proprietary file format created by Nintendo. The BFTTF namespace contains a single function, **`BFTTF::Decrypt`**.

This supports decrypting `.bfttf` files to `.ttf`, aswell as `.bfotf` files to `.otf`.

## Byaml

Byaml (Binary Yaml) is a proprietary file format created by Nintendo. This format is a Yaml file compiled as a binary format.

The Byaml class contains functions to parse and retrieve values from a `.byml` file.

ACNH uses [MurmurHash3](#murmurhash3) to hash a majority of column names in these files.

### ACNHByaml

The ACNHByaml class extends the Byaml class, adding one new function: `ACNHByaml::CalcOffsets`.

This function is for use with the save struct files found in `romfs:/System/Smmh`. The function allows programmatically calculating save addresses based on the specified `.byml` file. This enables a version agnostic method of getting save addresses at runtime.

## CRC32

The CRC32 namespace implements the [CRC32 Algorithm](https://wikipedia.org/wiki/Cyclic_redundancy_check) used by ACNH.

ACNH uses the polynomial `0x04C11DB7` for CRC32 operations, aswell as hashes within [BCSV](#bcsv) files. `consteval` functions are also provided for C++20 users.

## EncryptedInt

The EncryptedInt class implements the custom numerical cryptography that is used by ACNH.

ACNH uses this custom encryption for various numerical values, such as a Player's Money.

## MSBT

MSBT is a proprietary file format created by Nintendo. These files are used to store the game's text, as well as define how it's displayed.

The MSBT class contains functions to parse and retrieve strings from a `.msbt` file.

### ACNHItemMsbt

The ACNHItemMsbt class extends the MSBT class.

This class has one sole purpose, which is to fix Item Names from the various Item Name MSBT files. These files need to be 'fixed' as ACNH-specific MSBT Tags are usually before an Item's name, making it difficult to otherwise parse them.

## MurmurHash3

The MurmurHash3 namespace implements the [MurmurHash3 Algorithm](https://wikipedia.org/wiki/MurmurHash) used by ACNH.

ACNH uses MurmurHash3 for save checksums, as well as hashes within [Byaml](#byaml) files. A `consteval` function is also provided for C++20 users.

* A MurmurHash3 hashcat implementation can be found [here](https://github.com/Slattz/hashcat), as hash-type `94200`.

## SARC

SARC is a proprietary file format created by Nintendo. This format is an archive format, used for bundling files together.

The SARC class contains functions to parse and retrieve files from a `.sarc` file.

### ACNHSarc

The ACNHSarc class extends the SARC class. The main purpose of this class is to easily decompress and parse a `sarc.zs` file simultaneously. `sarc.zs` files appear throughout ACNH's filesystem.

## SaveCrypto

The SaveCrypto namespace implements the savefile cryptography that is used by ACNH.

ACNH uses 128bit [AES-CTR](https://wikipedia.org/wiki/Block_cipher_mode_of_operation#Counter_(CTR)) x-crypting, with a Key and Counter both generated from the respected save's Header file.

The **`SaveCrypto::Crypt`** function provides in-place encryption and decryption.

## sead::Random

The sead::Random class implements the random number generator used by ACNH. 

ACNH uses sead::Random for generating random values, aswell as the seeded function, **`sead::Random::init(u32 seed)`**, for [SaveCrypto](#savecrypto) use.
