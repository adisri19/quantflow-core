ThisBuild / scalaVersion := "3.3.3"
ThisBuild / version := "0.3.3"
ThisBuild / organization := "dev.quantflow"
ThisBuild / organizationName := "quantflow"
ThisBuild / organizationHomepage := Some(url("https://quantflow.io"))

ThisBuild / scmInfo := Some(
  ScmInfo(
    url("https://github.com/quantflow-io/quantflow"),
    "scm:git@github.com:quantflow-io/quantflow.git"
  )
)
ThisBuild / developers := List(
  Developer(
    id = "quantflow-io",
    name = "QuantFlow Team",
    email = "support@quantflow.io",
    url = url("https://github.com/quantflow-io")
  )
)

ThisBuild / description := "quantflow trading bot SDK for Scala"
ThisBuild / licenses := List("Apache-2.0" -> url("http://www.apache.org/licenses/LICENSE-2.0"))
ThisBuild / homepage := Some(url("https://github.com/quantflow-io/quantflow/tree/main/sdk/scala"))

// Publishing to GitHub Packages
ThisBuild / publishTo := Some("GitHub Package Registry" at "https://maven.pkg.github.com/quantflow-io/quantflow")
ThisBuild / credentials += Credentials(
  "GitHub Package Registry",
  "maven.pkg.github.com",
  "quantflow-io",
  sys.env.getOrElse("GITHUB_TOKEN", "")
)

lazy val root = (project in file("."))
  .settings(
    name := "quantflow-sdk",
    libraryDependencies ++= Seq(
      "org.scalatest" %% "scalatest" % "3.2.18" % Test
    )
  )
