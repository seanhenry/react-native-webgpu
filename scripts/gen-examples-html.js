#!/usr/bin/env node

const fs = require("fs/promises");
let file = "";
function println(str = "") {
  print(str + "\n");
}
function print(str = "") {
  file += str;
}

const run = async () => {
  println(`<html>
<head>
<title>Examples</title>
<style>
html, body {
  margin: 0;
  padding: 0;
  width: 100%;
}
.section {
}
.title {
  font-family: sans-serif;
  font-weight: 500;
  font-size: 24px;
  padding: 16px;
}
.items {
  width: 100%;
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
}
.item {
  width: 20%;
}
.nav {
  display: flex;
  gap: 8px;
}
</style>
</head>
<body>
<div>
<script>
function show(str) {
  const all = document.querySelectorAll('.section');
  all.forEach(el => {el.style.display = 'none'});
  const el = document.querySelector(str);
  el.style.display = 'block';
}
</script>
  `);
  const rnVersions = ["0.77.1", "0.76.7"];
  const sections = (
    await Promise.allSettled(
      rnVersions
        .flatMap((version) => {
          const versionId = version.replaceAll(".", "-");
          return [
            { id: `v${versionId}-ios-oldarch`, dir: `${version}/ios/oldarch` },
            { id: `v${versionId}-ios-newarch`, dir: `${version}/ios/newarch` },
            {
              id: `v${versionId}-android-oldarch`,
              dir: `${version}/android/oldarch`,
            },
            {
              id: `v${versionId}-android-newarch`,
              dir: `${version}/android/newarch`,
            },
          ];
        })
        .map(async (section) => ({
          ...section,
          files: await fs.readdir(section.dir),
        }))
    )
  )
    .filter(
      (result) => result.status === "fulfilled" && result.value.files.length > 0
    )
    .map((result) => result.value);

  println("<div class='nav'>");
  sections.forEach(({ id }) => {
    println(`<button onClick="show('#${id}')">${id}</button>`);
  });
  println("</div>");

  for (const { id, dir, files } of sections) {
    println(`<div id="${id}" class="section">`);
    println(`<div class="title">${id}</div>`);
    println(`<div class="items">`);
    files.forEach((file) => {
      const name = file.replace(/.\w+$/, "");
      println("<div class='item'>");
      print("<div>");
      print(name);
      print("</div>");
      println(`<img src="${dir}/${file}" alt="${name}" width="100%">`);
      println("</div>");
    });
    println("</div>");
    println("</div>");
  }

  println(`
</div>
</body>
</html>
`);
  console.log(file);
};

run();
