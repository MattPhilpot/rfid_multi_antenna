/*
 * This file is part of nmealib.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Extended descriptions of sentences are taken from
 *   http://www.gpsinformation.org/dale/nmea.htm
 */

#ifndef __NMEALIB_GNGNS_H__
#define __NMEALIB_GNGNS_H__

#include <nmealib/info.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

/** The NMEA prefix */
#define NMEALIB_GNGNS_PREFIX "GNGNS" //GNSS position fix from more than one constellation (eg. GPS + GLONASS)
//#define NMEALIB_GPGNS_PREFIX "GLGNS" //Information specific to the GLONASS constellation when more than one constellation is used for the differential fix
//#define NMEALIB_GLGNS_PREFIX "GPGNS" //Information specific to the GPS constellation when more than one constellation is used for the differential fix

/**
 * GNGNS packet information structure (Global Positioning System Fix Data)
 *
 * Essential fix data which provide 3D location and accuracy data.
 *
 * <pre>
 * $--GNS,time(UTC),latitude,ns,longitude,ew,mm,satellites,hdop,aa,geoidal,dgps age, dgps id*checksum
 * </pre>
 *
 * | Field       | Description                                            | present        |
 * | :---------: | ------------------------------------------------------ | :------------: |
 * | $GNGNS      | NMEA prefix                                            | -              |
 * | time        | Fix time (UTC) (5)                                     | UTCTIME        |
 * | latitude    | Latitude, in NDEG (DDMM.MMMMM)                         | LAT (1)        |
 * | ns          | North or South ('N' or 'S')                            | LAT (1)        |
 * | longitude   | Longitude, in NDEG (DDDMM.MMMMM)                       | LON (2)        |
 * | ew          | East or West ('E' or 'W')                              | LON (2)        |
 * | mm          | Mode indicator/signal quality                          | SIG (5)        |
 * | satellites  | Number of satellites being tracked                     | SATINVIEWCOUNT |
 * | hdop        | Horizontal dilution of position                        | HDOP           |
 * | aa          | Antenna altitude, in meters                            | ELV (3)        |
 * | geoidal     | Geoidal seperation, in meters                          | ELV (3)        |
 * | dgps age    | Time since last DGPS update, in seconds                | - (4)          |
 * | dgps id     | DGPS station ID number                                 | - (4)          |
 * | nav status  | Navigational status                                    | - (6)          |
 * | checksum    | NMEA checksum                                          | -              |

 *
 * (1) These fields are both required for a valid latitude<br/>
 * (2) These fields are both required for a valid longitude<br/>
 * (3) These fields are both required for a valid elevation<br/>
 * (4) Not supported yet<br/>
 * (5) 1st character is for GPS, 2nd for GLONASS<br/>
 * (6) S = SAFE, C = CAUTION, U = UNSAFE, V = NOT VALID<br/>
 *
 * Example:
 *
 * <pre>
 * $GNGNS
 * </pre>
 *
 * Note that if the height of geoid is missing then the elevation should be
 * suspect. Some non-standard implementations report elevation with respect to
 * the ellipsoid rather than geoid elevation. Some units do not report negative
 * elevations at all. This is the only sentence that reports elevation.
 */
typedef struct _NmeaGNGNS {
  uint32_t      present;
  NmeaTime       utc;
  double        latitude;
  char          latitudeNS;
  double        longitude;
  char          longitudeEW;
  NmeaModeIndicator   mode[2];
  unsigned int inViewCount;
  double        hdop;
  double        antennaAltitude;
  double        geoidalSeperation;
  double        dgpsAge;
  unsigned int dgpsSid;
  NmeaNavigationStatus navStatus;
} NmeaGNGNS;

/**
 * Parse a GNGNS sentence
 *
 * @param s The sentence
 * @param sz The length of the sentence
 * @param pack Where the result should be stored
 * @return True on success
 */
bool nmeaGNGNSParse(const char *s, const size_t sz, NmeaGNGNS *pack);

/**
 * Update an unsanitised NmeaInfo structure from a GNGNS packet structure
 *
 * @param pack The GNGNS packet structure
 * @param info The unsanitised NmeaInfo structure
 */
void nmeaGNGNSToInfo(const NmeaGNGNS *pack, NmeaInfo *info);

/**
 * Convert a sanitised NmeaInfo structure into a NmeaGNGNS structure
 *
 * @param info The sanitised NmeaInfo structure
 * @param pack The NmeaGNGNS structure
 */
void nmeaGNGNSFromInfo(const NmeaInfo *info, NmeaGNGNS *pack);

/**
 * Generate a GNGNS sentence
 *
 * @param s The buffer to generate the sentence in
 * @param sz The size of the buffer
 * @param pack The NmeaGNGNS structure
 * @return The length of the generated sentence; less than zero on failure,
 * larger than sz when the size of the buffer is too small to generate the
 * sentence in
 */
size_t nmeaGNGNSGenerate(char *s, const size_t sz, const NmeaGNGNS *pack);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* __NMEALIB_GNGNS_H__ */
