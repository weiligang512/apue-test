/***************************************************************************
 *   Copyright (c) 2009 Max Howell <max@last.fm>                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "../src/SmartPointerList.h"
#include <QtTest>

// use a macro, as we don't want to test copy ctor early
#define THREE_TIMERS( x ) SmartPointerList<QTimer> x; x << new QTimer << new QTimer << new QTimer



class TestSmartPointerList : public QObject
{
    Q_OBJECT
    
private slots:
    void testCount()
    {
        THREE_TIMERS( objects );
        QCOMPARE( objects.count(), 3 );
    }
    
    void testCopy()
    {
        THREE_TIMERS( objects1 );
        SmartPointerList<QTimer> objects2 = objects1;
        
        for (int x = 0; x < 3; ++x)
            QVERIFY( objects1[x] == objects2[x] );

        QCOMPARE( objects1.count(), 3 );
        QCOMPARE( objects2.count(), 3 );
        delete objects1.last();
        QCOMPARE( objects1.count(), 2 );
        QCOMPARE( objects2.count(), 2 );
    }
    
    void testCopyAndThenDelete()
    {
        THREE_TIMERS( os1 );
        SmartPointerList<QTimer>* os2 = new SmartPointerList<QTimer>( os1 );
        SmartPointerList<QTimer> os3( *os2 );
        
        delete os2;
        
        QCOMPARE( os1.count(), 3 );        
        QCOMPARE( os3.count(), 3 );
        
        delete os1[1];

        QCOMPARE( os1.count(), 2 );        
        QCOMPARE( os3.count(), 2 );
    }
    
    void testRemove()
    {
        THREE_TIMERS( objects );
        delete objects.last();
        QCOMPARE( objects.count(), 2 );
    }
    
    void testRemoveAt()
    {
        THREE_TIMERS( os );
        QTimer* t = os[1];
        os.removeAt( 1 );
        os << t;
        QCOMPARE( os.count(), 3 );
        delete t;
        QCOMPARE( os.count(), 2 );
    }

    void testMultipleOrgasms()
    {
        THREE_TIMERS( os );
        for (int x = 0; x < 10; ++x)
            os << os.last();
        QCOMPARE( os.count(), 13 );
        delete os.last();
        QCOMPARE( os.count(), 2 );
    }
    
    void testForeach()
    {
        THREE_TIMERS( objects );
        int x = 0;
        foreach (QTimer* o, objects) {
            (void) o;
            x++;
        }
        QCOMPARE( x, 3 );
    }
    
    void testOperatorPlus()
    {
        THREE_TIMERS( os1 );
        SmartPointerList<QTimer> os2 = os1;
        
        QCOMPARE( (os1 + os2).count(), 6 );
        delete os1.last();
        QCOMPARE( (os1 + os2).count(), 4 );
    }
    
    void testOperatorPlusEquals()
    {
        THREE_TIMERS( os );
        os += os;
        os += os;
        QCOMPARE( os.count(), 12 );
        QTimer* t = os.takeLast();
        QCOMPARE( os.count(), 11 );
        delete t;
        QCOMPARE( os.count(), 8 );
    }
};


QTEST_MAIN( TestSmartPointerList )
#include "TestSmartPointerList.moc"
