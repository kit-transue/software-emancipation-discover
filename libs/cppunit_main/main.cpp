/*
 * From the CppUnit Cookbook.  (Copyright implied but not asserted:
 * http://cppunit.sourceforge.net/doc/lastest/cppunit_cookbook.html)
 *
 * Original version by Michael Feathers. Doxygen conversion and update
 * by Baptiste Lepilleur.
 */

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main( int argc, char **argv)
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry
        = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool wasSuccessful = runner.run( "", false );
    return wasSuccessful ? 0 : 1;
}
