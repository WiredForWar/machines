#pragma once

namespace CrashDump
{

// Act on a --crash-test=<kind> argument, if there is one, by provoking the
// failure it names. Returns false when the argument is absent, so that start-up
// carries on; does not return at all when it is present and understood.
//
// The kinds are segv, abort, terminate, stack-overflow and hang everywhere, and
// invalid-parameter on Windows. The last of those is not a crash: it arms the
// watchdog and then starves it, so that the hang report can be checked by the
// same means as the crash ones. An unrecognised one exits with status 2 rather
// than starting the game, so that a typo is not mistaken for a passing check.
//
// This is deliberately available in a release build. A trigger that exists only
// in a developer build cannot answer the question it is for -- whether the
// binary people actually run reports its crashes -- and it doubles as the
// quickest way to confirm that a person reporting a crash can produce a report
// at all.
bool runCrashTestIfRequested(int argc, char* argv[]);

} // namespace CrashDump
