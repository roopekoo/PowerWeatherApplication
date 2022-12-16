/*
	Info about creating tests:
	https://doc.qt.io/qt-5/qtest-overview.html
*/

#include "datalinesaver.hh"

#include "utils.hh"

#include <QDebug>
#include <QtTest>
#include <random>

namespace pwa {

class DataLineSaverTest: public QObject {
	Q_OBJECT

  public:
	DataLineSaverTest() = default;
	~DataLineSaverTest() = default;

  private slots:
	//	will be called after the last test function was executed.
	//	https://doc.qt.io/qt-5/qtest-overview.html
	void cleanupTestCase();

	void initTestCase();

	void testCase0();
	void testCase1();
	void testCase2();

  private:
	const QString SAVE_DIRECTORY = "saved-data-sets/";

	const QString TEST_FILENAME0 = "testkalakukko101";
	const QString TEST_FILENAME1 = "testkalakukko-101.json";
	const QString TEST_FILENAME2 = "testpalapala 332";

	const bool AUTOMATICALLY_REMOVE_CREATED_FILES = true;

	void test(const DataLine& testInput, const QString& filename) const;
	void compare(const DataLine& dl1, const DataLine& dl2) const;
	void removeCreatedFile(const QString& filename) const;
};

void DataLineSaverTest::cleanupTestCase() {
	if (AUTOMATICALLY_REMOVE_CREATED_FILES) {
		removeCreatedFile(SAVE_DIRECTORY + TEST_FILENAME0);
		removeCreatedFile(SAVE_DIRECTORY + TEST_FILENAME1);
		removeCreatedFile(SAVE_DIRECTORY + TEST_FILENAME2);
	}
}

void DataLineSaverTest::initTestCase() {
	QDir dir{};
	dir.mkpath(SAVE_DIRECTORY);
}

void DataLineSaverTest::testCase0() {
	DataLine dataLine{Provider::UNSET, DataType::UNSET, TimeSpan{},
			std::vector<DataPoint>{}, QLatin1String(""), QLatin1String("")};

	test(dataLine, TEST_FILENAME0);
}

void DataLineSaverTest::testCase1() {
	DataLine dataLine{Provider::FINGRID, DataType::EL_PROD,
			TimeSpan{utils::fromIsoString("2021-03-13T21:37:00+02:00"),
					utils::fromIsoString("2022-04-15T22:38:01+03:11")},
			std::vector<DataPoint>{
					DataPoint{}, DataPoint{QDateTime{}, 4.022311}},
			QStringLiteral("lollokaa aatti"), QStringLiteral("yuniitti")};

	test(dataLine, TEST_FILENAME1);
}

void DataLineSaverTest::testCase2() {
	DataLine dataLine{Provider::FMI, DataType::NUCLEAR_PWR_PROD,
			TimeSpan{utils::fromIsoString("2022-04-15T22:38:01+03:11"),
					utils::fromIsoString("2021-03-13T21:37:00+02:00")},
			std::vector<DataPoint>{}, QStringLiteral("eiköhän tämä riitä"),
			QStringLiteral("jahka sanoo pahka")};

	const unsigned int ITERATIONS = 100000;
	dataLine.dataPoints.reserve(ITERATIONS);

	QDateTime dateTime = utils::fromIsoString("2019-04-13T21:27:00+03:00");

	std::random_device randomDevice{};
	std::default_random_engine randomEngine{randomDevice()};
	std::uniform_real_distribution<double> realDistribution{
			std::numeric_limits<double>::min(),
			std::numeric_limits<double>::max()};

	for (unsigned int i = 0; i < ITERATIONS; ++i) {
		dataLine.dataPoints.push_back(
				DataPoint{dateTime, realDistribution(randomEngine)});
	}

	test(dataLine, TEST_FILENAME2);
}

void DataLineSaverTest::test(
		const DataLine& testInput, const QString& filename) const {
	QString filePath = (SAVE_DIRECTORY + filename);
	DataLineSaver dataLineSaver{filePath};
	Q_ASSERT(!QFile::exists(filePath));
	dataLineSaver.write(testInput);

	QVERIFY(QFile::exists(filePath));

	DataLine readData{};
	dataLineSaver.readTo(readData);
	compare(testInput, readData);
}

void DataLineSaverTest::compare(
		const DataLine& dl1, const DataLine& dl2) const {
	QVERIFY(dl1.provider == dl2.provider);
	QVERIFY(dl1.dataType == dl2.dataType);
	QVERIFY(dl1.location == dl2.location);
	QVERIFY(dl1.yUnit == dl2.yUnit);
	QVERIFY(dl1.timeSpan.start == dl2.timeSpan.start);
	QVERIFY(dl1.timeSpan.end == dl2.timeSpan.end);

	QVERIFY(dl1.dataPoints.size() == dl2.dataPoints.size());
	for (std::vector<DataPoint>::size_type i = 0; i < dl1.dataPoints.size();
			++i) {
		DataPoint dp1 = dl1.dataPoints.at(i);
		DataPoint dp2 = dl2.dataPoints.at(i);
		QVERIFY(dp1.x == dp2.x);
		QVERIFY(dp1.y == dp2.y);
	}
}

void DataLineSaverTest::removeCreatedFile(const QString& filePath) const {
	const bool FILE_REMOVED_SUCCESSFULLY = QFile{filePath}.remove();
	Q_ASSERT(FILE_REMOVED_SUCCESSFULLY);
}

}  // namespace pwa

QTEST_APPLESS_MAIN(pwa::DataLineSaverTest)

#include "datalinesavertest.moc"
