import fs from 'node:fs/promises';
import path from 'node:path';

let file = '';
function println(str = '') {
  print(str + '\n');
}
function print(str = '') {
  file += str;
}

export async function generateExamplesHtml({
  outDir,
  rnVersions,
  architectures,
  platforms,
}) {
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
  const sections = (
    await Promise.allSettled(
      rnVersions
        .flatMap(version => {
          const versionId = version.replaceAll('.', '-');
          return platforms.flatMap(platform =>
            architectures.map(architecture => ({
              id: `v${versionId}-${platform}-${architecture}`,
              dir: `${outDir}/${version}/${platform}/${architecture}`,
            })),
          );
        })
        .map(async section => ({
          ...section,
          files: await fs.readdir(section.dir),
        })),
    )
  )
    .filter(
      result => result.status === 'fulfilled' && result.value.files.length > 0,
    )
    .map(result => result.value);

  println("<div class='nav'>");
  sections.forEach(({id}) => {
    println(`<button onClick="show('#${id}')">${id}</button>`);
  });
  println('</div>');

  for (const {id, dir, files} of sections) {
    println(`<div id="${id}" class="section">`);
    println(`<div class="title">${id}</div>`);
    println(`<div class="items">`);
    files.forEach(file => {
      const name = file.replace(/.\w+$/, '');
      println("<div class='item'>");
      print('<div>');
      print(name);
      print('</div>');
      println(`<img src="${dir}/${file}" alt="${name}" width="100%">`);
      println('</div>');
    });
    println('</div>');
    println('</div>');
  }

  println(`
</div>
</body>
</html>
`);
  return file;
}
