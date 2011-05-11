/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

#include <libpc/Iterator.hpp>
#include <libpc/PointBuffer.hpp>
#include <libpc/SchemaLayout.hpp>
#include <libpc/drivers/las/Reader.hpp>
#include <libpc/filters/CacheFilter.hpp>
#include "Support.hpp"

using namespace libpc;

BOOST_AUTO_TEST_SUITE(LasReaderTest)


BOOST_AUTO_TEST_CASE(test_sequential)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 3);
    
    libpc::SequentialIterator* iter = reader.createSequentialIterator();

    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p0_p1_p2(data, schema);
    }

    // Can we seek it? Yes, we can!
    iter->skip(97);
    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p100_p101_p102(data, schema);
    }

    delete iter;

    return;
}


BOOST_AUTO_TEST_CASE(test_random)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 3);
    
    libpc::RandomIterator* iter = reader.createRandomIterator();

    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p0_p1_p2(data, schema);
    }

    // Can we seek it? Yes, we can!
    iter->seek(100);
    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p100_p101_p102(data, schema);
    }

    // Can we seek to beginning? Yes, we can!
    iter->seek(0);
    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p0_p1_p2(data, schema);
    }
    
    delete iter;

    return;
}


BOOST_AUTO_TEST_CASE(test_random_laz)
{
    return; // BUG
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.laz"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 3);
    
    libpc::RandomIterator* iter = reader.createRandomIterator();

    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p0_p1_p2(data, schema);
    }

    // Can we seek it? Yes, we can!
    iter->seek(100);
    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p100_p101_p102(data, schema);
    }

    // Can we seek to beginning? Yes, we can!
    iter->seek(0);
    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 3);

        Support::check_p0_p1_p2(data, schema);
    }
    
    delete iter;

    return;
}


BOOST_AUTO_TEST_CASE(test_two_iters)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    BOOST_CHECK(reader.getNumPoints() == 1065);
    PointBuffer data(layout, 1065);

    {
        libpc::SequentialIterator* iter = reader.createSequentialIterator();
        BOOST_CHECK(iter->getIndex() == 0);

        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 1065);
        BOOST_CHECK(iter->getIndex() == 1065);

        Support::check_p0_p1_p2(data, schema);

        delete iter;
    }

    {
        libpc::RandomIterator* iter = reader.createRandomIterator();
        BOOST_CHECK(iter->getIndex() == 0);

        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 1065);
        BOOST_CHECK(iter->getIndex() == 1065);
        
        Support::check_p0_p1_p2(data, schema);

        delete iter;
    }

    return;
}


BOOST_AUTO_TEST_CASE(test_two_iters_with_cache)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    BOOST_CHECK(reader.getNumPoints() == 1065);
    BOOST_CHECK(355 * 3 == 1065);

    libpc::filters::CacheFilter cache(reader, 1, 355);
    BOOST_CHECK(cache.getNumPoints() == 1065);

    const Schema& schema = cache.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 355);

    boost::uint32_t numRead;

    {
        libpc::SequentialIterator* iter = cache.createSequentialIterator();
        BOOST_CHECK(iter->getIndex() == 0);

        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);

        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 710);

        Support::check_p355_p356_p357(data, schema);

        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 1065);

        Support::check_p710_p711_p712(data, schema);

        delete iter;
    }

    {
        libpc::RandomIterator* iter = cache.createRandomIterator();
        BOOST_CHECK(iter->getIndex() == 0);

        // read the middle third
        iter->seek(355);
        BOOST_CHECK(iter->getIndex() == 355);
        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 710);

        Support::check_p355_p356_p357(data, schema);

        // read the first third
        iter->seek(0);
        BOOST_CHECK(iter->getIndex() == 0);
        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);

        // read the first third again
        iter->seek(0);
        BOOST_CHECK(iter->getIndex() == 0);
        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);

        // read the last third
        iter->seek(710);
        BOOST_CHECK(iter->getIndex() == 710);
        numRead = iter->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iter->getIndex() == 1065);

        Support::check_p710_p711_p712(data, schema);

        delete iter;
    }

    return;
}


BOOST_AUTO_TEST_CASE(test_simultaneous_iters)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));
    BOOST_CHECK(reader.getDescription() == "Las Reader");

    BOOST_CHECK(reader.getNumPoints() == 1065);
    BOOST_CHECK(355 * 3 == 1065);

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 355);

    boost::uint32_t numRead;

    libpc::SequentialIterator* iterS1 = reader.createSequentialIterator();
    BOOST_CHECK(iterS1->getIndex() == 0);

    libpc::SequentialIterator* iterS2 = reader.createSequentialIterator();
    BOOST_CHECK(iterS2->getIndex() == 0);

    libpc::RandomIterator* iterR1 = reader.createRandomIterator();
    BOOST_CHECK(iterR1->getIndex() == 0);

    libpc::RandomIterator* iterR2 = reader.createRandomIterator();
    BOOST_CHECK(iterR2->getIndex() == 0);

    {
        numRead = iterS1->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterS1->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);
    }

    {
        iterS2->skip(355);

        numRead = iterS2->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterS2->getIndex() == 710);

        Support::check_p355_p356_p357(data, schema);
    }

    {
        iterR1->seek(355);
        numRead = iterR1->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterR1->getIndex() == 710);

        Support::check_p355_p356_p357(data, schema);
    }

    {
        iterR2->seek(0);
        numRead = iterR2->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterR2->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);
    }

    {
        iterS1->skip(355);
        numRead = iterS1->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterS1->getIndex() == 1065);

        Support::check_p710_p711_p712(data, schema);
    }

    {
        iterS2->skip(0);

        numRead = iterS2->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterS2->getIndex() == 1065);

        Support::check_p710_p711_p712(data, schema);
    }

    {
        iterR1->seek(355);
        numRead = iterR1->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterR1->getIndex() == 710);

        Support::check_p355_p356_p357(data, schema);
    }

    {
        iterR2->seek(710);
        numRead = iterR2->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterR2->getIndex() == 1065);

        Support::check_p710_p711_p712(data, schema);
    }

    {
        iterR1->seek(0);
        numRead = iterR1->read(data);
        BOOST_CHECK(numRead == 355);
        BOOST_CHECK(iterR1->getIndex() == 355);

        Support::check_p0_p1_p2(data, schema);
    }

    delete iterS1;
    delete iterS2;
    delete iterR1;
    delete iterR2;

    return;
}

BOOST_AUTO_TEST_CASE(test_iterator_checks)
{
    libpc::drivers::las::LasReader reader(Support::datapath("1.2-with-color.las"));

    BOOST_CHECK_EQUAL(reader.supportsIterator(StageIterator_Sequential), true);
    BOOST_CHECK_EQUAL(reader.supportsIterator(StageIterator_Random) , true);

    return;
}

static void test_a_format(const std::string& file, boost::uint8_t majorVersion, boost::uint8_t minorVersion, int pointFormat,
                              double xref, double yref, double zref, double tref, boost::uint16_t rref,  boost::uint16_t gref,  boost::uint16_t bref)
{
    libpc::drivers::las::LasReader reader(Support::datapath(file));

    BOOST_CHECK(reader.getPointFormat() == pointFormat);
    BOOST_CHECK(reader.getVersionMajor() == majorVersion);
    BOOST_CHECK(reader.getVersionMinor() == minorVersion);

    const Schema& schema = reader.getSchema();
    SchemaLayout layout(schema);

    PointBuffer data(layout, 1);
    
    libpc::SequentialIterator* iter = reader.createSequentialIterator();

    {
        boost::uint32_t numRead = iter->read(data);
        BOOST_CHECK(numRead == 1);

        Support::check_pN(data, schema, 0, xref, yref, zref, tref, rref, gref, bref);
    }

    delete iter;
}

BOOST_AUTO_TEST_CASE(test_different_formats)
{
    test_a_format("1.0_0.las", 1, 0, 0, 470692.440000, 4602888.900000, 16.000000, 0, 0, 0, 0);
    test_a_format("1.0_1.las", 1, 0, 1, 470692.440000, 4602888.900000, 16.000000, 1205902800.000000, 0, 0, 0);
    
    test_a_format("1.1_0.las", 1, 1, 0, 470692.440000, 4602888.900000, 16.000000, 0, 0, 0, 0);
    test_a_format("1.1_1.las", 1, 1, 1, 470692.440000, 4602888.900000, 16.000000, 1205902800.000000, 0, 0, 0);

    test_a_format("1.2_0.las", 1, 2, 0, 470692.440000, 4602888.900000, 16.000000, 0, 0, 0, 0);
    test_a_format("1.2_1.las", 1, 2, 1, 470692.440000, 4602888.900000, 16.000000, 1205902800.000000, 0, 0, 0);
    test_a_format("1.2_2.las", 1, 2, 2, 470692.440000, 4602888.900000, 16.000000, 0, 255, 12, 234);
    test_a_format("1.2_3.las", 1, 2, 3, 470692.440000, 4602888.900000, 16.000000, 1205902800.000000, 255, 12, 234);

    return;
}


BOOST_AUTO_TEST_CASE(test_vlr)
{
    libpc::drivers::las::LasReader reader(Support::datapath("lots_of_vlr.las"));

    BOOST_CHECK(reader.getVLRs().size() == 390);

    return;
}


BOOST_AUTO_TEST_SUITE_END()
