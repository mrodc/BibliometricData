//
// Created by mrodc on 10/20/25.
//

#ifndef SERVERSOCKET_SIMULATOR_H
#define SERVERSOCKET_SIMULATOR_H
#include "globalHeaders.h"

class Simulator
{
public:
    Simulator()
    {

        mthFunction.insert({"Sine", &Simulator::Sine});
        mthFunction.insert({"Cosine", &Simulator::Cosine});
        mthFunction.insert({"Square", &Simulator::Square});

        groupVector = {};
        styleVector = {};
    };

    //~Simulator();
    tuple<vector<float>, vector<float>> dataTuple;

    std::vector<std::string> groupVector;
    std::vector<std::string> styleVector;


    float Duration = 4;
    float SampleRate = 100;
    float Frequency = 10;
    float DCycle = 0.5;
    float AngularFrequency = 1.0;

    float Period = 0.5;
    float OnTime = 1.0;
    float Amplitude = 20;
    std::string Wave = "Sine";

    virtual void doTheMath() {

    };

    typedef float (Simulator::*mathFunction)(float value);
    std::map<std::string, mathFunction> mthFunction;

    float Sine(float value)
    {
        return Amplitude * sin(value*AngularFrequency);

    }
    float Cosine(float value)
    {

        return Amplitude * cos(value*AngularFrequency);
    }

    float Square(float value)
    {
        double time_in_period = fmod(value, Period); // Time within the current period

        auto val = time_in_period < OnTime ? Amplitude : 0.0;

        return val;
    }

    mathFunction getFunction(std::string name, bool *ok)
    {
        std::string nme = "none";
        mathFunction ptr = nullptr;

        *ok = false;
        auto p = mthFunction.find(name);
        if (p != mthFunction.end())
        {
            ptr = p->second;
            *ok = true;
        }
        return ptr;
    }

    json dataToJson()
    {
        auto x = get<0>(dataTuple);
        auto y = get<1>(dataTuple);

        std::vector<float> z(y);

        auto it = ranges::min_element(x);
        float xMin = *it;
        it = ranges::max_element(x);
        float xMax = *it;

        it = ranges::min_element(y);
        float yMin = *it;
        it = ranges::max_element(y);
        float yMax = *it;

        it = ranges::min_element(z);
        float zMin = *it;
        it = ranges::max_element(z);
        float zMax = *it;

        json options;
        options["xMin"] = xMin;
        options["xMax"] = xMax;
        options["yMin"] = yMin;
        options["yMax"] = yMax;
        options["zMin"] = yMin - 10;
        options["zMax"] = yMax + 10;
        options["style"] = "dot-color";

        json dataArray = json::array();
        int size = x.size();
        for (int i = 0; i < size; i++)
        {

            json point3d;
            point3d["x"] = x[i];
            point3d["y"] = y[i];
            point3d["z"] = z[i];

            if ( !groupVector.empty() && !styleVector.empty() ) {
                json sty;
                sty["fill"] = styleVector[i];
                sty["stroke"] = "#999";
                sty["size"] = 2;
                point3d["style"] = sty;
            }

            dataArray.push_back(point3d);
        }
        json tempJson;

        tempJson["data"] = dataArray;
        tempJson["options"] = options;

        // string jsonString = jsn.dump();
        return tempJson;
    };
    virtual ~Simulator()
    {
        std::cout << "Simulator destructor called." << std::endl;
    };

private:
};

// Derived class: Circle
class Bouncing : public Simulator
{
public:
    float v0;
    float theta;
    float g;

    float x_0;
    float y_0;
    float dt;
    int bounce;
    json dataJson;
    explicit Bouncing(json inJson)
    {
        auto data = inJson["Data"];
        std::string jsonString = data.dump(4);
        std::cout << jsonString << std::endl;
        v0 = 30.0;
        if (data.contains("v0"))
            v0 = data["v0"].get<float>();

        float tempTheta = 70.0;

        if (data.contains("theta"))
            tempTheta = data["theta"].get<float>();

        theta = tempTheta * static_cast<float>((M_PI / 180.0));

        g = 9.81;
        if (data.contains("gravity"))
            g = data["gravity"].get<float>();

        x_0 = 0.0;
        if (data.contains("x_0"))
            x_0 = data["x_0"].get<float>();

        y_0 = 0.0;
        if (data.contains("y_0"))
            y_0 = data["y_0"].get<float>();

        dt = 0.1;
        if (data.contains("dt"))
            dt = data["dt"].get<float>();
        bounce = 5;
        if (data.contains("bounce"))
            bounce = data["bounce"].get<int>();
    }

    tuple<float, float> simulation(float xi, float t, float e) const
    {
        auto x = [&](float xir, float t)
        {
            return xir + x_0 + v0 * cos(theta) * t;
        };

        auto y = [&](float tr, float e)
        {
            return y_0 + v0 * e * sin(theta) * tr - 0.5 * g * tr * tr;
        };

        return make_tuple(x(xi, t), y(t, e));
    };

    tuple<bool, float, float> ball(float xi, float t, float e) const
    {
        auto f = simulation(xi, t, e);
        if (get<1>(f) >= 0.0)
        {

            return make_tuple(true, get<0>(f), get<1>(f));
        }
        return make_tuple(false, get<0>(f), get<1>(f));
    };

    tuple<std::vector<float>, std::vector<float>> simulation() const
    {
        vector<float> x;
        vector<float> y;

        float xi = 0.0;
        int N = 0;

        while (N < bounce)
        {
            float t = 0.0;

            while (t < 10.0)
            // for (; t < 10; t = t + dt)
            {
                auto e = 1.0 / (N + 2.0);
                auto sim = ball(xi, t, e);

                if (get<2>(sim) >= 0)
                {
                    x.push_back(get<1>(sim));
                    y.push_back(get<2>(sim));
                }
                t = t + dt;
                if (get<0>(sim) == false)
                {

                    N += 1;
                    xi = get<1>(sim);
                    t = 100.0;
                }
            }
        }
        return make_tuple(x, y);
    };

    // Override the virtual draw function
    void doTheMath() override
    { // 'override' keyword is good practice for clarity and error checking
        try
        {
            auto doneMath = simulation();
            dataTuple = make_tuple(get<0>(doneMath), get<1>(doneMath));
            // dataJson = dataToJson(myTuple);
        }
        catch (...)
        {
        std:
            string error = "Error.";

            error += "DoneMath not called.";
        }
    }

    ~Bouncing()
    {
        std::cout << "Bouncing destructor called." << std::endl;
    }
};

class DemoClass : public Simulator
{

public:
    float duration = 4;
    float sampleRate = 100;
    float frequency = 10;
    float dCycle = 50.0;
    float angularFrequency;

    float amplitude = 20;
    std::string wave = "Sine";

    float period = 0;
    float onTime = 0;

    std::map<string, string> dotColor;

    explicit DemoClass(json inJson) {
        auto data = inJson["Data"];
        std::string jsonString = data.dump(4);

        if (data.contains("Duration"))
            Duration = duration = data["Duration"].get<float>();

        if (data.contains("SampleRate"))
            SampleRate = sampleRate = data["SampleRate"].get<float>();

        if (data.contains("Frequency")) {
            Frequency = frequency = data["Frequency"].get<float>();
            Period = period = 1/frequency;
        }

        if (data.contains("Amplitude"))
            Amplitude = amplitude = data["Amplitude"].get<float>();

        if (data.contains("DutyCycle")) {
            DCycle = dCycle = data["DutyCycle"].get<float>();
            OnTime = onTime = period * (dCycle / 100.0);

        }

        if (data.contains("Waveform"))
            Wave = wave = data["Waveform"].get<string>();

        // Calculate the angular frequency
        AngularFrequency = angularFrequency = 2.0 * M_PI * frequency;

        dotColor.insert({"blue", "blue"});
        dotColor.insert({"green", "green"});
        dotColor.insert({"red", "red"});
        dotColor.insert({"orange", "orange"});
    }

    void doTheMath() override
    { // 'override' keyword is good practice for clarity and error checking
        try
        {
            auto doneMath = generateWaveform();
            dataTuple = make_tuple(get<0>(doneMath), get<1>(doneMath));
            // dataJson = dataToJson(myTuple);
        }
        catch (...)
        {
        std::
            string error = "Error.";

            error += "DoneMath not called.";
        }
    }

    [[nodiscard]] tuple<std::vector<float>, std::vector<float>> generateWaveform()
    {
        std::vector<float> x;
        std::vector<float> y;
        std::vector<string> group;
        std::vector<string> color;

        std::vector<string> colorNames;
        std::transform(dotColor.begin(), dotColor.end(), std::back_inserter(colorNames),
                       [](const auto &pair)
                       { return pair.first; });

        std::vector<string> waves;

        // Split the string by spaces
        boost::split(waves, wave, boost::is_any_of(" ."), boost::token_compress_on);

        int waveIndex = 0;

        for (auto waveKey : waves)
        {
            wave = waveKey;

            bool ok = false;
            auto fnc = getFunction(wave, &ok);
            if (fnc == 0)
            {
                return make_tuple(x, y);
            }
            // Calculate the number of samples
            auto numSamples = duration * sampleRate;

            auto colorName = colorNames[waveIndex];
            waveIndex++;

            // Generate and output the waveform
            for (int i = 0; i < numSamples; i++)
            {
                double time = i / sampleRate;
                //double sample = (this->*fnc)(angularFrequency * time); // mthFunction[wave](angularFrequency * time);
                double sample = (this->*fnc)(time); // mthFunction[wave](angularFrequency * time);

                y.push_back(static_cast<float>(sample));
                x.push_back(static_cast<float>(i));
                groupVector.push_back(wave);
                styleVector.push_back(colorName);
            }
        }
        return make_tuple(x, y);
    }
};
#endif // SERVERSOCKET_SIMULATOR_H