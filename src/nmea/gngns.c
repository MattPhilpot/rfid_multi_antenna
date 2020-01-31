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

#include <nmealib/gngns.h>

#include <nmealib/context.h>
#include <nmealib/sentence.h>
#include <nmealib/util.h>
#include <nmealib/validate.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool nmeaGNGNSParse(const char *s, const size_t sz, NmeaGNGNS *pack) {
  size_t tokenCount;
  char timeBuf[16];

  if (!s //
      || !sz //
      || !pack) {
    return false;
  }

  nmeaContextTraceBuffer(s, sz);

  /* Clear before parsing, to be able to detect absent fields */
  *timeBuf = '\0';
  memset(pack, 0, sizeof(*pack));
  pack->latitude = NaN;
  pack->longitude = NaN;
  pack->mode[0] = '\0';
  pack->mode[1] = '\0';
  pack->inViewCount = UINT_MAX;
  pack->hdop = NaN;
  pack->antennaAltitude = NaN;
  pack->geoidalSeperation = NaN;
  pack->dgpsAge = NaN;
  pack->dgpsSid = UINT_MAX;

  /* parse */
  tokenCount = nmeaScanf(s, sz, //
      "$" NMEALIB_GNGNS_PREFIX ",%16s,%F,%C,%F,%C,%s,%u,%F,%f,%f,%F,%u,%C*", //
      timeBuf, //
      &pack->latitude, //
      &pack->latitudeNS, //
      &pack->longitude, //
      &pack->longitudeEW, //
      &pack->mode, //
      &pack->inViewCount, //
      &pack->hdop, //
      &pack->antennaAltitude, //
      &pack->geoidalSeperation, //
      &pack->dgpsAge, //
      &pack->dgpsSid,
      &pack->navStatus);

  /* see that there are enough tokens */
  if (tokenCount != 12) {
    nmeaContextError(NMEALIB_GNGNS_PREFIX " parse error: need 12 tokens, got %lu in '%s'", (long unsigned) tokenCount,
        s);
    goto err;
  }

  /* determine which fields are present and validate them */

  if (*timeBuf) {
    if (!nmeaTimeParseTime(timeBuf, &pack->utc) //
        || !nmeaValidateTime(&pack->utc, NMEALIB_GNGNS_PREFIX, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCTIME);
  } else {
    memset(&pack->utc, 0, sizeof(pack->utc));
  }

  if (!isNaN(pack->latitude)) {
    if (!nmeaValidateNSEW(pack->latitudeNS, true, NMEALIB_GNGNS_PREFIX, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LAT);
  } else {
    pack->latitude = 0.0;
    pack->latitudeNS = '\0';
  }

  if (!isNaN(pack->longitude)) {
    if (!nmeaValidateNSEW(pack->longitudeEW, false, NMEALIB_GNGNS_PREFIX, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LON);
  } else {
    pack->longitude = 0.0;
    pack->longitudeEW = '\0';
  }

  if (pack->mode) {
    if (!nmeaValidateMode(pack->mode[0], NMEALIB_GNGNS_PREFIX, s) &&
        !nmeaValidateMode(pack->mode[1], NMEALIB_GNGNS_PREFIX, s)) {
      goto err;
    }

    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SIG);
  } else {
    pack->mode[0] = 'N';
    pack->mode[1] = 'N';
  }

  if (pack->inViewCount != UINT_MAX) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  } else {
    pack->inViewCount = 0;
  }

  if (!isNaN(pack->hdop)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HDOP);
  } else {
    pack->hdop = 0.0;
  }

  if (!isNaN(pack->antennaAltitude)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_ELV);
  } else {
    pack->antennaAltitude = 0.0;
  }

  if (!isNaN(pack->geoidalSeperation)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HEIGHT);
  } else {
    pack->geoidalSeperation = 0.0;
  }

  if (!isNaN(pack->dgpsAge)) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSAGE);
  } else {
    pack->dgpsAge = 0.0;
  }

  if (pack->dgpsSid != UINT_MAX) {
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSSID);
  } else {
    pack->dgpsSid = 0;
  }

  return true;

err:
  memset(pack, 0, sizeof(*pack));
  pack->mode[0] = 'N';
  pack->mode[1] = 'N';
  return false;
}

void nmeaGNGNSToInfo(const NmeaGNGNS *pack, NmeaInfo *info) {
  if (!pack //
      || !info) {
    return;
  }

  nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SMASK);

  info->smask |= NMEALIB_SENTENCE_GNGNS;

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCTIME)) {
    info->utc.hour = pack->utc.hour;
    info->utc.min = pack->utc.min;
    info->utc.sec = pack->utc.sec;
    info->utc.hsec = pack->utc.hsec;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_UTCTIME);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LAT)) {
    info->latitude = ((pack->latitudeNS == 'S') ?
        -pack->latitude :
        pack->latitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LAT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LON)) {
    info->longitude = ((pack->longitudeEW == 'W') ?
        -pack->longitude :
        pack->longitude);
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_LON);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SIG)) {
    info->mode[0] = pack->mode[0];
    info->mode[1] = pack->mode[1];
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SIG);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    info->satellites.inViewCount = pack->inViewCount;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HDOP)) {
    info->hdop = pack->hdop;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HDOP);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_ELV)) {
    info->elevation = pack->antennaAltitude;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_ELV);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HEIGHT)) {
    info->height = pack->geoidalSeperation;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_HEIGHT);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSAGE)) {
    info->dgpsAge = pack->dgpsAge;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSSID)) {
    info->dgpsSid = pack->dgpsSid;
    nmeaInfoSetPresent(&info->present, NMEALIB_PRESENT_DGPSSID);
  }
}

void nmeaGNGNSFromInfo(const NmeaInfo *info, NmeaGNGNS *pack) {
  if (!pack //
      || !info) {
    return;
  }

  memset(pack, 0, sizeof(*pack));

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_UTCTIME)) {
    pack->utc.hour = info->utc.hour;
    pack->utc.min = info->utc.min;
    pack->utc.sec = info->utc.sec;
    pack->utc.hsec = info->utc.hsec;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_UTCTIME);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LAT)) {
    pack->latitude = fabs(info->latitude);
    pack->latitudeNS = ((info->latitude >= 0.0) ?
        'N' :
        'S');
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LAT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_LON)) {
    pack->longitude = fabs(info->longitude);
    pack->longitudeEW = ((info->longitude >= 0.0) ?
        'E' :
        'W');
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_LON);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SIG)) {
    pack->mode[0] = info->mode[0];
    pack->mode[1] = info->mode[1];
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SIG);
  } else {
    pack->mode[0] = NMEALIB_MODE_NO_FIX;
    pack->mode[1] = NMEALIB_MODE_NO_FIX;
  }


  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    pack->inViewCount = info->satellites.inViewCount;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HDOP)) {
    pack->hdop = info->hdop;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HDOP);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_ELV)) {
    pack->antennaAltitude = info->elevation;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_ELV);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_HEIGHT)) {
    pack->geoidalSeperation = info->height;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_HEIGHT);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_DGPSAGE)) {
    pack->dgpsAge = info->dgpsAge;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSAGE);
  }

  if (nmeaInfoIsPresentAll(info->present, NMEALIB_PRESENT_DGPSSID)) {
    pack->dgpsSid = info->dgpsSid;
    nmeaInfoSetPresent(&pack->present, NMEALIB_PRESENT_DGPSSID);
  }
}

size_t nmeaGNGNSGenerate(char *s, const size_t sz, const NmeaGNGNS *pack) {

#define dst       (&s[chars])
#define available ((sz <= (size_t) chars) ? 0 : (sz - (size_t) chars))

  int chars = 0;

  if (!s //
      || !pack) {
    return 0;
  }

  chars += snprintf(dst, available, "$" NMEALIB_GNGNS_PREFIX);

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_UTCTIME)) {
    chars += snprintf(dst, available, //
        ",%02u%02u%02u.%02u", //
        pack->utc.hour, //
        pack->utc.min, //
        pack->utc.sec, //
        pack->utc.hsec);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LAT)) {
    chars += snprintf(dst, available, ",%09.4f", pack->latitude);
    if (pack->latitudeNS) {
      chars += snprintf(dst, available, ",%c", pack->latitudeNS);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_LON)) {
    chars += snprintf(dst, available, ",%010.4f", pack->longitude);
    if (pack->longitudeEW) {
      chars += snprintf(dst, available, ",%c", pack->longitudeEW);
    } else {
      chars += snprintf(dst, available, ",");
    }
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SIG)) {
    chars += snprintf(dst, available, ",%d", pack->mode[0]);
    chars += snprintf(dst, available, "%d", pack->mode[1]);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_SATINVIEWCOUNT)) {
    chars += snprintf(dst, available, ",%02u", pack->inViewCount);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HDOP)) {
    chars += snprintf(dst, available, ",%03.1f", pack->hdop);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_ELV)) {
    chars += snprintf(dst, available, ",%03.1f", pack->antennaAltitude);
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_HEIGHT)) {
    chars += snprintf(dst, available, ",%03.1f", pack->geoidalSeperation);
  } else {
    chars += snprintf(dst, available, ",,");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSAGE)) {
    chars += snprintf(dst, available, ",%03.1f", pack->dgpsAge);
  } else {
    chars += snprintf(dst, available, ",");
  }

  if (nmeaInfoIsPresentAll(pack->present, NMEALIB_PRESENT_DGPSSID)) {
    chars += snprintf(dst, available, ",%u", pack->dgpsSid);
  } else {
    chars += snprintf(dst, available, ",");
  }

  /* checksum */
  chars += nmeaAppendChecksum(s, sz, (size_t) chars);

  return (size_t) chars;

#undef available
#undef dst

}
