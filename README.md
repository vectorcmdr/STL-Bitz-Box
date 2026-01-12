<h1 align="center">
	🗃️ <br>
	STL Bitz Box
</h1>

<h3 align="center">
	Your personal STL library!
</h3>

<br>

<div align="center">
  
[![Stargazers][stars-shield]][stars-url] [![Issues][issues-shield]][issues-url] [![Contributors][contributors-shield]][contributors-url]

<!--
[![License][license-shield]][license-url] 
-->

</div>

[issues-shield]: https://img.shields.io/github/issues/vectorcmdr/stl-bitz-box?style=for-the-badge&logo=gitbook&color=fdac72&logoColor=f5f5ef&labelColor=2a262c
[issues-url]: https://github.com/vectorcmdr/stl-bitz-box/graphs/issues

[contributors-shield]: https://img.shields.io/github/contributors/vectorcmdr/stl-bitz-box?style=for-the-badge&logo=github&color=c5b4f8&logoColor=f5f5ef&labelColor=2a262c
[contributors-url]: https://github.com/vectorcmdr/stl-bitz-box/graphs/contributors

[stars-shield]: https://img.shields.io/github/stars/vectorcmdr/stl-bitz-box?style=for-the-badge&logo=starship&color=b9e48b&logoColor=f5f5ef&labelColor=2a262c
[stars-url]: https://github.com/vectorcmdr/stl-bitz-box/stargazers

<!--
[license-shield]: https://img.shields.io/github/license/vectorcmdr/stl-bitz-box?label=License&style=for-the-badge&logo=bookstack&color=49d1e9&logoColor=f5f5ef&labelColor=2a262c
[license-url]: https://github.com/vectorcmdr/stl-bitz-box/blob/main/LICENSE
-->

<h3 align="center">
  <p align="center">
    <a href="https://vectorcmdr.github.io/STL-Bitz-Box">Live Demo </a>
    &middot;
    <a href="https://github.com/vectorcmdr/STL-Bitz-Box/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    &middot;
    <a href="https://github.com/vectorcmdr/STL-Bitz-Box/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</h3>

<p align="center">
  <img src="img/preview.png" width="800"/>
</p>


## 🔍 Overview

**A simple STL library manager for Windows that helps you organize your huge collection of 3D prints from monthly model subs and website downloads.**

Features the [**senzai theme**](https://vectorcmdr.github.io/senzai-theme/).


## 🔨 Built With

<a href="#">
<img src="https://img.shields.io/badge/javascript-grey?style=for-the-badge&logo=javascript" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/C-grey?style=for-the-badge&logo=c" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/html-grey?style=for-the-badge&logo=html5" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/css-grey?style=for-the-badge&logo=css3" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/threedotjs-grey?style=for-the-badge&logo=threedotjs" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/bootstrap-grey?style=for-the-badge&logo=bootstrap" height="35"/></a>

<a href="#">
<img src="https://img.shields.io/badge/batch-grey?style=for-the-badge&logo=powershell" height="35"/></a>


## 📋 Goals

- Web based
- Thumbnail previews
- 3D viewer
- Searching and filtering
- Tag support and filtering
- Model license file support
- Original file link support
- Model readme support
- Local HTTP daemon option
- Hosts file redirect for custom URL
- Senzai theme

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## 🔧 Installation

1. Fork and clone the repo into your preferred folder locally.
2. Place your STL files into the `stl` folder according to the below file structure:

```bash
stl
  ├── Category
  │   ├── STL Pack Name
  │   │   ├── files
  │   │   │   ├── Actual STL file 1.stl
  │   │   │   └── Actual STL file 2.stl
  │   │   ├── LICENSE.txt
  │   │   ├── README.txt
  │   │	  └── TAGS.txt
  │   ├── STL Pack Name
  │   │	  └── ...
  └── Category
      └── ...
```

- `Category` folders can be whatever you want the main sorting root to be, such as their license type (such as _CC0_, _CC-BY_, etc.) or their purpose or genre (such as _"Tabletop"_, _"Household"_, _"Cosplay"_ - or _"Space Soldiers"_, _"Buff Green Aliens"_, _"Space Bugs"_, etc.)

- `STL Pack Name` folders below a category folder is the name of the download or release and the file structure supports Thingiverse format downloads immediately. They could be, for example: _"Space Storming Mecha Marine Men July Release"_ or _"Air conditioning unit scaled for 28mm tabletop terrain - 4974429"_.

- `files` folder contains the actual `.stl` files themselves. Thumbnail `.png` files will be generated here, or images can be manually added for each `.stl` file by adding a `.png` with the same name.

- `LICENSE.txt` is the license for the `.stl` files contained in this pack / folder. It is an optional file.

- `README.txt` is the readme for the `.stl` files contained in this pack / folder. It is an optional file. If you want a link available to the original files, or another relevant page then add it to this file at the end. Remove any additional links that start with 'http'.

- `TAGS.txt` is the tags for the `.stl` files contained in this pack / folder. It is an optional file. This will be ingested by the setup and added to each `.stl` in the files folder within STL Bitz Box for searching. This file should be filled out by the user with the tags they want for this pack.

> [!TIP]
> The [**live demo directories in the repo**](https://github.com/vectorcmdr/STL-Bitz-Box/blob/main/stl/) illustrate how this can look using some CC0 files downloaded from thingiverse, with some tags added.

3. Run `setup_fast.bat` to install the server.

4. `setup_advanced.bat` can be used instead by advanced users. It allows you to individually:
   - Install the STL thumbnail dependencies
   - Install the server daemon and hosts redirect url
   - Convert OBJ files to STL
   - Rebuild thumbnials and file DB
   - Rebuild thumbnails only
   - Rebuild file DB only
   - Audit the size of the library for files at or above 50MB & 100MB
   - Audit the library for FBX or OBJ files

5. `setup_fast.bat` will automatically install dependencies, the server and build the thumbs and DB then launch.

6. To update the library post initial setup with any changes or additions, run `update.bat` - or to individually control each step you can also use `setup_advanced.bat`.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## 🚀 Usage

1. To access STL Bitz Box, navigate to `http://stlbitzbox.local`  in your browser. If it fails to load for some reason, make sure that the HTTP server daemon (`STLBitzBoxServer.exe`) is running and has not been blocked or removed from autorun.

2. Hover over the preview thumbnail under `PREVIEW` to enlarge it.

3. You can click relevant links for each STL:
	- Clicking the `.STL` link under `TOOLS` will download the `.stl` file.
	- Clicking the `3D VIEW` link under `TOOLS` will load a new browser window with a 3D viewer for the model with pan/zoom/rotate and support for shaded materials or normal mapping.
	- Clicking the `README` link under `TOOLS` will load the readme file if present.
	- Clicking the `LINK` link under `TOOLS` will load the external link from the readme file if present.
	- Clicking the container name link under `CONTAINER` will load the license file if present.

4. Use the filter options below the table headers to further filter your selection or to search.
	- Use the drop down under `CONTAINER` to filter by container name.
	- Use the search filter field under `MODEL DIRECTORY` to search/filter for words in the pack name folders below the containers.
	- Use the search filter field under `MESH DIRECTORY` to search/filter for words in the mesh files (STLs) in the pack directory.
	- Use the search filter field under `TAGS` to search/filter for words in the tags files.
	- Use the sorting arrows on each header to sort by ascending/descending order. The default order is the ingestion order (windows default name sorting).

> [!TIP]
> The [**live demo**](https://vectorcmdr.github.io/STL-Bitz-Box/) allows you to play around with this searching/filtering to see how it works.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### ⌨️ Advanced Use
Advanced users can utilise the batch files to build thumbs and the DB files + html index and then host the resulting build online by serving index.html.
> [!TIP]
> Keep in mind how your STL files license agreements may apply. Do not host content publically that you aren't authorized to share!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### 🧱 Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

#### 🤸 Top contributors:

<a href="https://github.com/vectorcmdr/stl-bitz-box/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=vectorcmdr/stl-bitz-box" />
</a>

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### 📃 License

Distributed under the MIT license. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### 📨 Contact

Josh (vectorcmdr) - [u/vector_cmdr](https://www.reddit.com/user/vector_cmdr/)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### ☕ Support More Like This

<a href="https://ko-fi.com/vector_cmdr">
<img src="https://custom-icon-badges.demolab.com/badge/-Donate-lightblue?style=for-the-badge&logo=coffee&logoColor=red" height="64"/></a>

### 📜 Acknowledgments:

Project includes the following libraries/plugins:
* [Assimp](https://assimp.org/)
* [Papa's Best STL Thumbnails](https://papas-best.com/stlthumbnails_en)
* [Datatable](https://github.com/Holt59/datatable)
* [contrib.rocks](https://contrib.rocks)

<p align="right">(<a href="#readme-top">back to top</a>)</p>
