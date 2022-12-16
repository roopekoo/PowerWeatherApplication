/*
	Info about creating tests:
	https://doc.qt.io/qt-5/qtest-overview.html
*/

#include "preference.hh"

#include <QDebug>
#include <QtTest>

namespace pwa {

/*!
	\brief Namespace for storing preference constans
	for testing Preference.
*/
namespace test0 {
	inline const QString PLOT_GRAPH_TYPE = "plot plot";
	inline const QString CALCULATION_TYPE = "calc calc cilc calc";
	inline const QString WEATHER_PLACE = "Suomenmaa";
	inline const QDateTime FROMDATE = QDateTime::currentDateTime();
	inline const QDateTime CURRENT_DATE = FROMDATE.addYears(3);
	inline const QDateTime TODATE = CURRENT_DATE.addDays(12);
	inline const IdnameBoolUnorderedMap CHECKBOX_STATES{
			{"1", false}, {"asdf", true}, {"whatever", false}, {"false", true}};
}  // namespace test0

class PreferenceTest: public QObject {
	Q_OBJECT

  public:
	PreferenceTest();
	~PreferenceTest() = default;

  private slots:
	void initTestCase();

	void testCasePlotGraphType0();
	void testCaseCalculationType0();
	void testCaseWeatherPlace0();
	void testCaseTimeDateInterval0();
	void testCaseCheckBoxStates0();

	void testCaseHasDateTimeInterval0();

	//	will be called after the last test function was executed.
	//	https://doc.qt.io/qt-5/qtest-overview.html
	void cleanupTestCase();

  private:
	const QString SAVE_DIRECTORY = "saved-preferences/";
	const QString TEST_FILENAME0 = "preference-test-0.json";
	const QString FILEPATH0 = SAVE_DIRECTORY + TEST_FILENAME0;

	const bool AUTOMATICALLY_REMOVE_CREATED_FILES = true;

	Preference readPreference_;

	void initPreference0(Preference& preference);

	void removeCreatedFile(const QString& filename) const;
};

PreferenceTest::PreferenceTest(): readPreference_{FILEPATH0} {
}

void PreferenceTest::initTestCase() {
	QDir dir{};
	dir.mkpath(SAVE_DIRECTORY);

	{
		Preference preference{FILEPATH0};
		initPreference0(preference);

		Q_ASSERT(!QFile::exists(FILEPATH0));
		bool writeSuccess = preference.write();
		Q_ASSERT(writeSuccess);
		QVERIFY(QFile::exists(FILEPATH0));
	}

	bool readSuccess = readPreference_.read();
	Q_ASSERT(readSuccess);
}

void PreferenceTest::testCasePlotGraphType0() {
	QCOMPARE(readPreference_.plotGraphType(), test0::PLOT_GRAPH_TYPE);
}

void PreferenceTest::testCaseCalculationType0() {
	QCOMPARE(readPreference_.calculationType(), test0::CALCULATION_TYPE);
}

void PreferenceTest::testCaseWeatherPlace0() {
	QCOMPARE(readPreference_.weatherPlace(), test0::WEATHER_PLACE);
}

void PreferenceTest::testCaseTimeDateInterval0() {
	QCOMPARE(readPreference_.fromDate(test0::CURRENT_DATE), test0::FROMDATE);
	QCOMPARE(readPreference_.toDate(test0::CURRENT_DATE), test0::TODATE);
}

void PreferenceTest::testCaseCheckBoxStates0() {
	QCOMPARE(readPreference_.checkBoxStates(), test0::CHECKBOX_STATES);
}

void PreferenceTest::testCaseHasDateTimeInterval0() {
	QVERIFY(readPreference_.hasDateTimeInterval() ==
			(readPreference_.fromDate(test0::CURRENT_DATE) !=
					readPreference_.toDate(test0::CURRENT_DATE)));
}

void PreferenceTest::cleanupTestCase() {
	if (AUTOMATICALLY_REMOVE_CREATED_FILES) {
		removeCreatedFile(SAVE_DIRECTORY + TEST_FILENAME0);
	}
}

void PreferenceTest::initPreference0(Preference& preference) {
	preference.setPlotGraphType(test0::PLOT_GRAPH_TYPE);
	preference.setCalculationType(test0::CALCULATION_TYPE);
	preference.setWeatherPlace(test0::WEATHER_PLACE);
	preference.setDateTimeInterval(
			test0::FROMDATE, test0::CURRENT_DATE, test0::TODATE);
	preference.setCheckBoxStates(test0::CHECKBOX_STATES);
}

void PreferenceTest::removeCreatedFile(const QString& filePath) const {
	const bool FILE_REMOVED_SUCCESSFULLY = QFile{filePath}.remove();
	Q_ASSERT(FILE_REMOVED_SUCCESSFULLY);
}

}  // namespace pwa

QTEST_APPLESS_MAIN(pwa::PreferenceTest)

#include "preferencetest.moc"
