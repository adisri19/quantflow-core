name := "sma-bot"
version := "1.0.3"
scalaVersion := "3.3.1"

// GitHub Packages resolver for quantflow SDK
resolvers += "GitHub Package Registry" at "https://maven.pkg.github.com/quantflow-io/quantflow"
credentials += Credentials(
  "GitHub Package Registry",
  "maven.pkg.github.com",
  "quantflow-io",
  sys.env.getOrElse("GITHUB_TOKEN", "")
)

libraryDependencies ++= Seq(
  "dev.quantflow" %% "quantflow-sdk" % "0.3.3",
  "com.softwaremill.sttp.client3" %% "core" % "3.9.0",
  "io.circe" %% "circe-core" % "0.14.6",
  "io.circe" %% "circe-parser" % "0.14.6",
  "io.circe" %% "circe-generic" % "0.14.6"
)

// Exclude raw version from compilation
Compile / unmanagedSources / excludeFilter := "Main_raw.scala"

assembly / assemblyMergeStrategy := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case x => MergeStrategy.first
}

assembly / mainClass := Some("Main")
