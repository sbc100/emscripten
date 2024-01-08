/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

assert(!STANDALONE_WASM, "library_time.js should not be included in standalone mode");

addToLibrary({
  _mktime_js__i53abi: true,
  _mktime_js__deps: ['$ydayFromDate'],
  _mktime_js: (tmPtr) => {
    var date = new Date({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);

    // There's an ambiguous hour when the time goes back; the tm_isdst field is
    // used to disambiguate it.  Date() basically guesses, so we fix it up if it
    // guessed wrong, or fill in tm_isdst with the guess if it's -1.
    var dst = {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'i32') }}};
    var guessedOffset = date.getTimezoneOffset();
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dstOffset = Math.min(winterOffset, summerOffset); // DST is in December in South
    if (dst < 0) {
      // Attention: some regions don't have DST at all.
      {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'Number(summerOffset != winterOffset && dstOffset == guessedOffset)', 'i32') }}};
    } else if ((dst > 0) != (dstOffset == guessedOffset)) {
      var nonDstOffset = Math.max(winterOffset, summerOffset);
      var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
      // Don't try setMinutes(date.getMinutes() + ...) -- it's messed up.
      date.setTime(date.getTime() + (trueOffset - guessedOffset)*60000);
    }

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};
    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    // To match expected behavior, update fields from date
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getYear()', 'i32') }}};

    var timeMs = date.getTime();
    if (isNaN(timeMs)) {
      return -1;
    }
    // Return time in microseconds
    return timeMs / 1000;
  },

  _gmtime_js__i53abi: true,
  _gmtime_js: (time, tmPtr) => {
    var date = new Date(time * 1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getUTCSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getUTCMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getUTCHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getUTCDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getUTCMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getUTCFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
  },

  _timegm_js__i53abi: true,
  _timegm_js: (tmPtr) => {
    var time = Date.UTC({{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}} + 1900,
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
                        {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
                        0);
    var date = new Date(time);

    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getUTCDay()', 'i32') }}};
    var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
    var yday = ((date.getTime() - start) / (1000 * 60 * 60 * 24))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};

    return date.getTime() / 1000;
  },

  _localtime_js__i53abi: true,
  _localtime_js__deps: ['$ydayFromDate'],
  _localtime_js: (time, tmPtr) => {
    var date = new Date(time*1000);
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'date.getSeconds()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_min, 'date.getMinutes()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'date.getHours()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'date.getDate()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'date.getMonth()', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_year, 'date.getFullYear()-1900', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'date.getDay()', 'i32') }}};

    var yday = ydayFromDate(date)|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_yday, 'yday', 'i32') }}};
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_gmtoff, '-(date.getTimezoneOffset() * 60)', LONG_TYPE) }}};

    // Attention: DST is in December in South, and some regions don't have DST at all.
    var start = new Date(date.getFullYear(), 0, 1);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();
    var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset))|0;
    {{{ makeSetValue('tmPtr', C_STRUCTS.tm.tm_isdst, 'dst', 'i32') }}};
  },

  // musl-internal function used to implement both `asctime` and `asctime_r`
  __asctime_r: (tmPtr, buf) => {
    var date = {
      tm_sec: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_sec, 'i32') }}},
      tm_min: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_min, 'i32') }}},
      tm_hour: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_hour, 'i32') }}},
      tm_mday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mday, 'i32') }}},
      tm_mon: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_mon, 'i32') }}},
      tm_year: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_year, 'i32') }}},
      tm_wday: {{{ makeGetValue('tmPtr', C_STRUCTS.tm.tm_wday, 'i32') }}}
    };
    var days = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
    var months = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];
    var s = days[date.tm_wday] + ' ' + months[date.tm_mon] +
        (date.tm_mday < 10 ? '  ' : ' ') + date.tm_mday +
        (date.tm_hour < 10 ? ' 0' : ' ') + date.tm_hour +
        (date.tm_min < 10 ? ':0' : ':') + date.tm_min +
        (date.tm_sec < 10 ? ':0' : ':') + date.tm_sec +
        ' ' + (1900 + date.tm_year) + "\n";

    // asctime_r is specced to behave in an undefined manner if the algorithm would attempt
    // to write out more than 26 bytes (including the null terminator).
    // See http://pubs.opengroup.org/onlinepubs/9699919799/functions/asctime.html
    // Our undefined behavior is to truncate the write to at most 26 bytes, including null terminator.
    stringToUTF8(s, buf, 26);
    return buf;
  },

  _tzset_js__deps: ['$stringToUTF8',
#if ASSERTIONS
    '$lengthBytesUTF8',
#endif
  ],
  _tzset_js__internal: true,
  _tzset_js: (timezone, daylight, std_name, dst_name) => {
    // TODO: Use (malleable) environment variables instead of system settings.
    var currentYear = new Date().getFullYear();
    var winter = new Date(currentYear, 0, 1);
    var summer = new Date(currentYear, 6, 1);
    var winterOffset = winter.getTimezoneOffset();
    var summerOffset = summer.getTimezoneOffset();

    // Local standard timezone offset. Local standard time is not adjusted for
    // daylight savings.  This code uses the fact that getTimezoneOffset returns
    // a greater value during Standard Time versus Daylight Saving Time (DST).
    // Thus it determines the expected output during Standard Time, and it
    // compares whether the output of the given date the same (Standard) or less
    // (DST).
    var stdTimezoneOffset = Math.max(winterOffset, summerOffset);

    // timezone is specified as seconds west of UTC ("The external variable
    // `timezone` shall be set to the difference, in seconds, between
    // Coordinated Universal Time (UTC) and local standard time."), the same
    // as returned by stdTimezoneOffset.
    // See http://pubs.opengroup.org/onlinepubs/009695399/functions/tzset.html
    {{{ makeSetValue('timezone', '0', 'stdTimezoneOffset * 60', POINTER_TYPE) }}};

    {{{ makeSetValue('daylight', '0', 'Number(winterOffset != summerOffset)', 'i32') }}};

    var extractZone = (timezoneOffset) => {
      // Why inverse sign?
      // Read here https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/getTimezoneOffset
      var sign = timezoneOffset >= 0 ? "-" : "+";

      var absOffset = Math.abs(timezoneOffset)
      var hours = String(Math.floor(absOffset / 60)).padStart(2, "0");
      var minutes = String(absOffset % 60).padStart(2, "0");

      return `UTC${sign}${hours}${minutes}`;
    }

    var winterName = extractZone(winterOffset);
    var summerName = extractZone(summerOffset);
#if ASSERTIONS
    assert(winterName);
    assert(summerName);
    assert(lengthBytesUTF8(winterName) <= {{{ cDefs.TZNAME_MAX }}}, `timezone name truncated to fit in TZNAME_MAX (${winterName})`);
    assert(lengthBytesUTF8(summerName) <= {{{ cDefs.TZNAME_MAX }}}, `timezone name truncated to fit in TZNAME_MAX (${summerName})`);
#endif
    if (summerOffset < winterOffset) {
      // Northern hemisphere
      stringToUTF8(winterName, std_name, {{{ cDefs.TZNAME_MAX + 1 }}});
      stringToUTF8(summerName, dst_name, {{{ cDefs.TZNAME_MAX + 1 }}});
    } else {
      stringToUTF8(winterName, dst_name, {{{ cDefs.TZNAME_MAX + 1 }}});
      stringToUTF8(summerName, std_name, {{{ cDefs.TZNAME_MAX + 1 }}});
    }
  },

  $MONTH_DAYS_REGULAR: [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_LEAP: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  $MONTH_DAYS_REGULAR_CUMULATIVE: [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
  $MONTH_DAYS_LEAP_CUMULATIVE: [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335],

  $isLeapYear: (year) => year%4 === 0 && (year%100 !== 0 || year%400 === 0),

  $ydayFromDate__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP_CUMULATIVE', '$MONTH_DAYS_REGULAR_CUMULATIVE'],
  $ydayFromDate: (date) => {
    var leap = isLeapYear(date.getFullYear());
    var monthDaysCumulative = (leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE);
    var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1; // -1 since it's days since Jan 1

    return yday;
  },

  $arraySum: (array, index) => {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]) {
      // no-op
    }
    return sum;
  },

  $addDays__deps: ['$isLeapYear', '$MONTH_DAYS_LEAP', '$MONTH_DAYS_REGULAR'],
  $addDays: (date, days) => {
    var newDate = new Date(date.getTime());
    while (days > 0) {
      var leap = isLeapYear(newDate.getFullYear());
      var currentMonth = newDate.getMonth();
      var daysInCurrentMonth = (leap ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[currentMonth];

      if (days > daysInCurrentMonth-newDate.getDate()) {
        // we spill over to next month
        days -= (daysInCurrentMonth-newDate.getDate()+1);
        newDate.setDate(1);
        if (currentMonth < 11) {
          newDate.setMonth(currentMonth+1)
        } else {
          newDate.setMonth(0);
          newDate.setFullYear(newDate.getFullYear()+1);
        }
      } else {
        // we stay in current month
        newDate.setDate(newDate.getDate()+days);
        return newDate;
      }
    }

    return newDate;
  },
});
