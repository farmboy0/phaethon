/* Phaethon - A FLOSS resource explorer for BioWare's Aurora engine games
 *
 * Phaethon is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * Phaethon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * Phaethon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phaethon. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Unit tests for our RIM file archive class.
 */

#include "gtest/gtest.h"

#include "src/common/error.h"
#include "src/common/memreadstream.h"

#include "src/aurora/rimfile.h"

// Percy Bysshe Shelley's "Ozymandias"
static const char *kFileData =
	"I met a traveller from an antique land\n"
	"Who said: Two vast and trunkless legs of stone\n"
	"Stand in the desert. Near them, on the sand,\n"
	"Half sunk, a shattered visage lies, whose frown,\n"
	"And wrinkled lip, and sneer of cold command,\n"
	"Tell that its sculptor well those passions read\n"
	"Which yet survive, stamped on these lifeless things,\n"
	"The hand that mocked them and the heart that fed:\n"
	"And on the pedestal these words appear:\n"
	"'My name is Ozymandias, king of kings:\n"
	"Look on my works, ye Mighty, and despair!'\n"
	"Nothing beside remains. Round the decay\n"
	"Of that colossal wreck, boundless and bare\n"
	"The lone and level sands stretch far away.";

// Percy Bysshe Shelley's "Ozymandias", within a RIM file
static const byte kRIMFile[] = {
	0x52,0x49,0x4D,0x20,0x56,0x31,0x2E,0x30,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
	0x14,0x00,0x00,0x00,0x6F,0x7A,0x79,0x6D,0x61,0x6E,0x64,0x69,0x61,0x73,0x00,0x00,
	0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x34,0x00,0x00,0x00,
	0x6F,0x02,0x00,0x00,0x49,0x20,0x6D,0x65,0x74,0x20,0x61,0x20,0x74,0x72,0x61,0x76,
	0x65,0x6C,0x6C,0x65,0x72,0x20,0x66,0x72,0x6F,0x6D,0x20,0x61,0x6E,0x20,0x61,0x6E,
	0x74,0x69,0x71,0x75,0x65,0x20,0x6C,0x61,0x6E,0x64,0x0A,0x57,0x68,0x6F,0x20,0x73,
	0x61,0x69,0x64,0x3A,0x20,0x54,0x77,0x6F,0x20,0x76,0x61,0x73,0x74,0x20,0x61,0x6E,
	0x64,0x20,0x74,0x72,0x75,0x6E,0x6B,0x6C,0x65,0x73,0x73,0x20,0x6C,0x65,0x67,0x73,
	0x20,0x6F,0x66,0x20,0x73,0x74,0x6F,0x6E,0x65,0x0A,0x53,0x74,0x61,0x6E,0x64,0x20,
	0x69,0x6E,0x20,0x74,0x68,0x65,0x20,0x64,0x65,0x73,0x65,0x72,0x74,0x2E,0x20,0x4E,
	0x65,0x61,0x72,0x20,0x74,0x68,0x65,0x6D,0x2C,0x20,0x6F,0x6E,0x20,0x74,0x68,0x65,
	0x20,0x73,0x61,0x6E,0x64,0x2C,0x0A,0x48,0x61,0x6C,0x66,0x20,0x73,0x75,0x6E,0x6B,
	0x2C,0x20,0x61,0x20,0x73,0x68,0x61,0x74,0x74,0x65,0x72,0x65,0x64,0x20,0x76,0x69,
	0x73,0x61,0x67,0x65,0x20,0x6C,0x69,0x65,0x73,0x2C,0x20,0x77,0x68,0x6F,0x73,0x65,
	0x20,0x66,0x72,0x6F,0x77,0x6E,0x2C,0x0A,0x41,0x6E,0x64,0x20,0x77,0x72,0x69,0x6E,
	0x6B,0x6C,0x65,0x64,0x20,0x6C,0x69,0x70,0x2C,0x20,0x61,0x6E,0x64,0x20,0x73,0x6E,
	0x65,0x65,0x72,0x20,0x6F,0x66,0x20,0x63,0x6F,0x6C,0x64,0x20,0x63,0x6F,0x6D,0x6D,
	0x61,0x6E,0x64,0x2C,0x0A,0x54,0x65,0x6C,0x6C,0x20,0x74,0x68,0x61,0x74,0x20,0x69,
	0x74,0x73,0x20,0x73,0x63,0x75,0x6C,0x70,0x74,0x6F,0x72,0x20,0x77,0x65,0x6C,0x6C,
	0x20,0x74,0x68,0x6F,0x73,0x65,0x20,0x70,0x61,0x73,0x73,0x69,0x6F,0x6E,0x73,0x20,
	0x72,0x65,0x61,0x64,0x0A,0x57,0x68,0x69,0x63,0x68,0x20,0x79,0x65,0x74,0x20,0x73,
	0x75,0x72,0x76,0x69,0x76,0x65,0x2C,0x20,0x73,0x74,0x61,0x6D,0x70,0x65,0x64,0x20,
	0x6F,0x6E,0x20,0x74,0x68,0x65,0x73,0x65,0x20,0x6C,0x69,0x66,0x65,0x6C,0x65,0x73,
	0x73,0x20,0x74,0x68,0x69,0x6E,0x67,0x73,0x2C,0x0A,0x54,0x68,0x65,0x20,0x68,0x61,
	0x6E,0x64,0x20,0x74,0x68,0x61,0x74,0x20,0x6D,0x6F,0x63,0x6B,0x65,0x64,0x20,0x74,
	0x68,0x65,0x6D,0x20,0x61,0x6E,0x64,0x20,0x74,0x68,0x65,0x20,0x68,0x65,0x61,0x72,
	0x74,0x20,0x74,0x68,0x61,0x74,0x20,0x66,0x65,0x64,0x3A,0x0A,0x41,0x6E,0x64,0x20,
	0x6F,0x6E,0x20,0x74,0x68,0x65,0x20,0x70,0x65,0x64,0x65,0x73,0x74,0x61,0x6C,0x20,
	0x74,0x68,0x65,0x73,0x65,0x20,0x77,0x6F,0x72,0x64,0x73,0x20,0x61,0x70,0x70,0x65,
	0x61,0x72,0x3A,0x0A,0x27,0x4D,0x79,0x20,0x6E,0x61,0x6D,0x65,0x20,0x69,0x73,0x20,
	0x4F,0x7A,0x79,0x6D,0x61,0x6E,0x64,0x69,0x61,0x73,0x2C,0x20,0x6B,0x69,0x6E,0x67,
	0x20,0x6F,0x66,0x20,0x6B,0x69,0x6E,0x67,0x73,0x3A,0x0A,0x4C,0x6F,0x6F,0x6B,0x20,
	0x6F,0x6E,0x20,0x6D,0x79,0x20,0x77,0x6F,0x72,0x6B,0x73,0x2C,0x20,0x79,0x65,0x20,
	0x4D,0x69,0x67,0x68,0x74,0x79,0x2C,0x20,0x61,0x6E,0x64,0x20,0x64,0x65,0x73,0x70,
	0x61,0x69,0x72,0x21,0x27,0x0A,0x4E,0x6F,0x74,0x68,0x69,0x6E,0x67,0x20,0x62,0x65,
	0x73,0x69,0x64,0x65,0x20,0x72,0x65,0x6D,0x61,0x69,0x6E,0x73,0x2E,0x20,0x52,0x6F,
	0x75,0x6E,0x64,0x20,0x74,0x68,0x65,0x20,0x64,0x65,0x63,0x61,0x79,0x0A,0x4F,0x66,
	0x20,0x74,0x68,0x61,0x74,0x20,0x63,0x6F,0x6C,0x6F,0x73,0x73,0x61,0x6C,0x20,0x77,
	0x72,0x65,0x63,0x6B,0x2C,0x20,0x62,0x6F,0x75,0x6E,0x64,0x6C,0x65,0x73,0x73,0x20,
	0x61,0x6E,0x64,0x20,0x62,0x61,0x72,0x65,0x0A,0x54,0x68,0x65,0x20,0x6C,0x6F,0x6E,
	0x65,0x20,0x61,0x6E,0x64,0x20,0x6C,0x65,0x76,0x65,0x6C,0x20,0x73,0x61,0x6E,0x64,
	0x73,0x20,0x73,0x74,0x72,0x65,0x74,0x63,0x68,0x20,0x66,0x61,0x72,0x20,0x61,0x77,
	0x61,0x79,0x2E
};

GTEST_TEST(RIMFile, getNameHashAlgo) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	EXPECT_EQ(rim.getNameHashAlgo(), Common::kHashNone);
}

GTEST_TEST(RIMFile, getResources) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	const Aurora::RIMFile::ResourceList &resources = rim.getResources();
	ASSERT_EQ(resources.size(), 1);

	const Aurora::RIMFile::Resource &resource = *resources.begin();

	EXPECT_STREQ(resource.name.c_str(), "ozymandias");
	EXPECT_EQ(resource.type, Aurora::kFileTypeTXT);
	EXPECT_EQ(resource.hash, 0);
	EXPECT_EQ(resource.index, 0);
}

GTEST_TEST(RIMFile, getResourceSize) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	EXPECT_EQ(rim.getResourceSize(0), strlen(kFileData));

	EXPECT_THROW(rim.getResourceSize(1), Common::Exception);
}

GTEST_TEST(RIMFile, findResourceHash) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	EXPECT_EQ(rim.findResource(0), 0xFFFFFFFF);
}

GTEST_TEST(RIMFile, findResourceName) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	EXPECT_EQ(rim.findResource("ozymandias", Aurora::kFileTypeTXT), 0);

	EXPECT_EQ(rim.findResource("ozymandias", Aurora::kFileTypeBMP), 0xFFFFFFFF);
	EXPECT_EQ(rim.findResource("nope"      , Aurora::kFileTypeTXT), 0xFFFFFFFF);
	EXPECT_EQ(rim.findResource("nope"      , Aurora::kFileTypeBMP), 0xFFFFFFFF);
}

GTEST_TEST(RIMFile, getResource) {
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(kRIMFile);
	const Aurora::RIMFile rim(stream);

	Common::SeekableReadStream *file = rim.getResource(0);
	ASSERT_NE(file, static_cast<Common::SeekableReadStream *>(0));

	ASSERT_EQ(file->size(), strlen(kFileData));

	for (size_t i = 0; i < strlen(kFileData); i++)
		EXPECT_EQ(file->readByte(), kFileData[i]) << "At index " << i;

	delete file;
}
