const { Jimp } = require('jimp');
const fs = require('fs');
const path = require('path');

const dir = 'screenshots';

async function convertAll() {
  if (!fs.existsSync(dir)) {
    console.error(`Directory '${dir}' does not exist.`);
    return;
  }
  const files = fs.readdirSync(dir);
  for (const file of files) {
    if (file.endsWith('.bmp')) {
      const bmpPath = path.join(dir, file);
      const pngPath = bmpPath.replace('.bmp', '.png');
      console.log(`Converting ${bmpPath} to ${pngPath}...`);
      try {
        const image = await Jimp.read(bmpPath);
        await image.write(pngPath);
        fs.unlinkSync(bmpPath);
        console.log(`Successfully converted ${file} to PNG.`);
      } catch (err) {
        console.error(`Failed to convert ${file}:`, err);
      }
    }
  }
}

convertAll();
