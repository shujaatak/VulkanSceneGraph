/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/AnimationGroup.h>
#include <vsg/animation/Joint.h>
#include <vsg/animation/TransformSampler.h>
#include <vsg/app/Camera.h>
#include <vsg/core/compare.h>
#include <vsg/io/Input.h>
#include <vsg/io/Options.h>
#include <vsg/io/Output.h>
#include <vsg/nodes/MatrixTransform.h>

using namespace vsg;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TransformKeyframes
//
TransformKeyframes::TransformKeyframes()
{
}

void TransformKeyframes::read(Input& input)
{
    Object::read(input);

    input.read("name", name);

    // read position key frames
    uint32_t num_positions = input.readValue<uint32_t>("positions");
    positions.resize(num_positions);
    for (auto& position : positions)
    {
        input.matchPropertyName("position");
        input.read(1, &position.time);
        input.read(1, &position.value);
    }

    // read rotation key frames
    uint32_t num_rotations = input.readValue<uint32_t>("rotations");
    rotations.resize(num_rotations);
    for (auto& rotation : rotations)
    {
        input.matchPropertyName("rotation");
        input.read(1, &rotation.time);
        input.read(1, &rotation.value);
    }

    // read scale key frames
    uint32_t num_scales = input.readValue<uint32_t>("scales");
    scales.resize(num_scales);
    for (auto& scale : scales)
    {
        input.matchPropertyName("scale");
        input.read(1, &scale.time);
        input.read(1, &scale.value);
    }
}

void TransformKeyframes::write(Output& output) const
{
    Object::write(output);

    output.write("name", name);

    // write position key frames
    output.writeValue<uint32_t>("positions", positions.size());
    for (auto& position : positions)
    {
        output.writePropertyName("position");
        output.write(1, &position.time);
        output.write(1, &position.value);
        output.writeEndOfLine();
    }

    // write rotation key frames
    output.writeValue<uint32_t>("rotations", rotations.size());
    for (auto& rotation : rotations)
    {
        output.writePropertyName("rotation");
        output.write(1, &rotation.time);
        output.write(1, &rotation.value);
        output.writeEndOfLine();
    }

    // write scale key frames
    output.writeValue<uint32_t>("scales", scales.size());
    for (auto& scale : scales)
    {
        output.writePropertyName("scale");
        output.write(1, &scale.time);
        output.write(1, &scale.value);
        output.writeEndOfLine();
    }
}

template<typename T, typename V>
bool sample(double time, const T& values, V& value)
{
    if (values.size() == 0) return false;

    if (values.size() == 1)
    {
        value = values.front().value;
        return true;
    }

    auto pos_itr = values.begin();
    if (time <= pos_itr->time)
    {
        value = pos_itr->value;
        return true;
    }
    else
    {
        using value_type = typename T::value_type;
        pos_itr = std::lower_bound(values.begin(), values.end(), time, [](const value_type& elem, double t) -> bool { return elem.time < t; });

        if (pos_itr == values.begin())
        {
            value = values.front().value;
            return true;
        }

        if (pos_itr == values.end())
        {
            value = values.back().value;
            return true;
        }

        auto before_pos_itr = pos_itr - 1;
        double delta_time = (pos_itr->time - before_pos_itr->time);
        double r = delta_time != 0.0 ? (time - before_pos_itr->time) / delta_time : 0.5;

        value = mix(before_pos_itr->value, pos_itr->value, r);

        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TransformSampler
//
TransformSampler::TransformSampler() :
    position(0.0, 0.0, 0.0),
    rotation(),
    scale(1.0, 1.0, 1.0)
{
}

TransformSampler::TransformSampler(const TransformSampler& rhs, const CopyOp& copyop) :
    Inherit(rhs, copyop),
    keyframes(copyop(rhs.keyframes)),
    object(copyop(rhs.object)),
    position(rhs.position),
    rotation(rhs.rotation),
    scale(rhs.scale)
{
}

int TransformSampler::compare(const Object& rhs_object) const
{
    int result = AnimationSampler::compare(rhs_object);
    if (result != 0) return result;

    auto& rhs = static_cast<decltype(*this)>(rhs_object);
    if ((result = compare_pointer(keyframes, rhs.keyframes)) != 0) return result;
    if ((result = compare_pointer(object, rhs.object)) != 0) return result;
    if ((result = compare_value(position, rhs.position)) != 0) return result;
    if ((result = compare_value(rotation, rhs.rotation)) != 0) return result;
    return compare_value(scale, rhs.scale);
}

void TransformSampler::update(double time)
{
    if (keyframes)
    {
        sample(time, keyframes->positions, position);
        sample(time, keyframes->rotations, rotation);
        sample(time, keyframes->scales, scale);
    }

    if (object) object->accept(*this);
}

double TransformSampler::maxTime() const
{
    double maxTime = 0.0;
    if (keyframes)
    {
        if (!keyframes->positions.empty()) maxTime = std::max(maxTime, keyframes->positions.back().time);
        if (!keyframes->rotations.empty()) maxTime = std::max(maxTime, keyframes->rotations.back().time);
        if (!keyframes->scales.empty()) maxTime = std::max(maxTime, keyframes->scales.back().time);
    }

    return maxTime;
}

void TransformSampler::apply(mat4Value& matrix)
{
    matrix.set(mat4(transform()));
}

void TransformSampler::apply(dmat4Value& matrix)
{
    matrix.set(transform());
}

void TransformSampler::apply(MatrixTransform& mt)
{
    mt.matrix.set(transform());
}

void TransformSampler::apply(Joint& joint)
{
    joint.matrix.set(transform());
}

void TransformSampler::apply(LookAt& lookAt)
{
    lookAt.set(transform());
}

void TransformSampler::apply(Camera& camera)
{
    if (camera.viewMatrix)
    {
        camera.viewMatrix->accept(*this);
    }
}

void TransformSampler::read(Input& input)
{
    AnimationSampler::read(input);
    input.read("keyframes", keyframes);
    input.read("object", object);
}

void TransformSampler::write(Output& output) const
{
    AnimationSampler::write(output);
    output.write("keyframes", keyframes);
    output.write("object", object);
}
