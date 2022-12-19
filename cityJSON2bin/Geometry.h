#pragma once

class CityModel;

class Geometry
{
public:
    Geometry(CityModel& city) : m_city(city) {}

    void Convert(rapidjson::Value& jgeometry);

private:
    struct Point3D {
        double coord[3];
    };
    typedef std::list<Point3D> Loop;

private:
    void ConvertItem(rapidjson::Value& jitem);
    void ConvertMultiSurface(rapidjson::Value& boundaries);
    void ConvertSurface(rapidjson::Value& boundaries);
    void GetLoop(Loop& loop, rapidjson::Value& jloop);
    void GetPoint(Point3D& point, rapidjson::Value& jpoint);

private:
    CityModel&           m_city;
};

