/***************************************************************************
 * copyright            : (C) 2008 Daniel Jones <danielcjones@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#ifndef AMAROK_BIAS_H
#define AMAROK_BIAS_H

#include "Meta.h"
#include "collection/support/XmlQueryReader.h"

#include <QDomElement>
#include <QMutex>
#include <QObject>
#include <QSet>

class Collection;
class QueryMaker;
class XmlQueryWriter;

namespace PlaylistBrowserNS
{
    class BiasWidget;
}


namespace Dynamic
{

    /**
     * A bias is essentially just a function that evaluates the suitability of a
     * playlist in some arbitrary way.
     */
    class Bias
    {
        public:
            static Bias* fromXml( QDomElement );

            Bias();
            virtual ~Bias() {}

            QString description() const;
            void setDescription( const QString& );

            /**
             * Create a widget appropriate for editing the bias.
             */
            virtual PlaylistBrowserNS::BiasWidget* widget( QWidget* parent = 0 );

            /**
             * Returns xml suitable to be loaded with fromXml.
             */
            virtual QDomElement xml() const = 0;

            /**
             * Should the bias be considered or ignored by the playlist?
             */
            void setActive( bool );
            bool active();


            /**
             * Returns a value in the range [-1,1]. (The sign is not considered,
             * but it may be useful to return negative numbers for
             * implementing reevaluate.) Playlist generation is being
             * treated as a minimization problem, so 0 means the bias is completely
             * satisfied, (+/-)1 that it is not satisfied at all. The tracks that
             * precede the playlist are passed as 'context'.
             */
            virtual double energy( const Meta::TrackList& playlist, const Meta::TrackList& context ) = 0;


            /**
             * When a track is swapped in the playlist, avoid completely reevaluating
             * the energy function if possible.
             */
            virtual double reevaluate( double oldEnergy, const Meta::TrackList& oldPlaylist,
                    Meta::TrackPtr newTrack, int newTrackPos, const Meta::TrackList& context );

        protected:
            bool m_active;
            QString m_description;
    };



    /**
     * A bias that depends on the state of the collection.
     */
    class CollectionDependantBias : public QObject, public Bias
    {
        Q_OBJECT

        public:
            CollectionDependantBias();
            CollectionDependantBias( Amarok::Collection* );

            /**
             * This gets called when the collection changes. It's expected to
             * emit a biasUpdated signal when finished.
             */
            virtual void update() = 0;
            bool needsUpdating();

        signals:
            void biasUpdated( CollectionDependantBias* );

        public slots:
            void collectionUpdated();

        protected:
            Amarok::Collection* m_collection; // null => all queryable collections
            bool m_needsUpdating;
            QMutex m_mutex;
    };


    /**
     * This a bias in which the order and size of the playlist are not
     * considered. Instead we want a given proportion (weight) of the tracks to
     * have a certain property (or belong to a certain set).
     */
    class GlobalBias : public CollectionDependantBias
    {
        Q_OBJECT
                
        public:
            GlobalBias( double weight, XmlQueryReader::Filter );
            GlobalBias( Amarok::Collection* coll, double weight, XmlQueryReader::Filter query );
            
            ~GlobalBias();

            void setQuery( XmlQueryReader::Filter );

            QDomElement xml() const;

            PlaylistBrowserNS::BiasWidget* widget( QWidget* parent = 0 );
            const XmlQueryReader::Filter& filter() const;

            double energy( const Meta::TrackList& playlist, const Meta::TrackList& context );
            double reevaluate( double oldEnergy, const Meta::TrackList& oldPlaylist,
                    Meta::TrackPtr newTrack, int newTrackPos, const Meta::TrackList& context );

            const QSet<QByteArray>& propertySet() { return m_property; }
            bool trackSatisfies( Meta::TrackPtr );
            void update();

            double weight() const;
            void setWeight( double );


        private slots:
            void updateReady( QString collectionId, QStringList );
            void updateFinished();

        private:
            double m_weight; /// range: [0,1]
            QSet<QByteArray> m_property;
            XmlQueryWriter* m_qm;
            XmlQueryReader::Filter m_filter;

            // Disable copy constructor and assignment
            GlobalBias( const GlobalBias& );
            GlobalBias& operator= ( const GlobalBias& );
    };

    /**
     * A bias that works with numerical fields and attempts to fit the playlist to
     * a normal distribution.
     */
    class NormalBias : public Bias
    {
        public:
            NormalBias();

            QDomElement xml() const;
            PlaylistBrowserNS::BiasWidget* widget( QWidget* parent = 0 );

            double energy( const Meta::TrackList& playlist, const Meta::TrackList& context );

            /**
             * The mean of the distribution, the 'ideal' value.
             */
            void setValue( double );
            double value() const;

            /**
             * The QueryMaker field that will be considered. Only numerical
             * fields (e.g. year, score, etc) will work.
             */
            void setField( qint64 );
            qint64 field() const;

            /**
             * A number in [0.0,1.0] that controls the variance of the
             * distribution in a sort of contrived but user friendly way. 1.0 is
             * maximum strictness, 0.0 means minimum strictness.
             */
            void setScale( double );
            double scale();

        private:
            double sigmaFromScale( double scale );
            double releventField( Meta::TrackPtr t );
            void setDefaultMu();

            double m_scale;

            double m_mu;    //! mean
            double m_sigma; //! standard deviation

            qint64 m_field;
    };
}

Q_DECLARE_METATYPE( Dynamic::Bias* )

#endif

