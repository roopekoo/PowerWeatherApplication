#include "apitypes.hh"

namespace pwa {

QDebug operator<<(QDebug debug, const pwa::DataPoint& point) {
	QDebugStateSaver saver(debug);
	debug.nospace() << "x: "
					<< point.x.toOffsetFromUtc(point.x.offsetFromUtc())
							   .toString(Qt::ISODate)
					<< ", y: " << point.y;
	return debug;
}

QDebug operator<<(QDebug debug, const std::vector<pwa::DataPoint>& points) {
	QDebugStateSaver saver(debug);
	debug.noquote();
	debug.nospace() << "DataPoints(" << points.size() << "):\n";
	int i = 0;
	for (const pwa::DataPoint& point : points) {
		++i;
		debug.nospace() << point << '\n';

		if (i >= 15) {
			debug.nospace() << "... " << points.size() - i << " more\n";
			break;
		}
	}
	return debug;
}

}  // namespace pwa
