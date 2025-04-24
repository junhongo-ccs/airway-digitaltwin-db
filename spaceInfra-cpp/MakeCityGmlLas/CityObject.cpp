#include "CityObject.h"
#include "GeoPolygon.h"

CityObject::CityObject() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());
}

void CityObject::addPolygon(GeoPolygon* pPolygon) {
	polygonContainer.addObjects(1, pPolygon);
	pPolygon->setPolygonId(polygonContainer.getObjectCount() - 1);

}
