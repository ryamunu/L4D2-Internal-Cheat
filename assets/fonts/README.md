# AstolfoWare UI Fonts & Assets

This directory contains documentation and metadata about the fonts integrated into the premium visual redesign (Inspired by yzs/lime style).

## Fonts Integrated
- **MuseoFont**: A premium, clean sans-serif font used for all primary UI text and headers. High legibility and modern aesthetic matching high-end products.
- **FontAwesome 5 Solid**: Integrated for navigation icons. Provides the visual cues for the Top Bar navigation system.

## Implementation Details
- Fonts are embedded directly into the binary via compiled byte arrays in `Features/NewMenu/Fonts.h`.
- `F_Regular`: Size 14.0px, used for options and labels.
- `F_Title`: Size 16.0px (Semi-bold weight), used for headers and active states.
- `F_Icons`: FontAwesome icons with glyph merging, size 18.0px.

## Licences
- **Museo Sans**: Used under standard commercial license for software embedding.
- **FontAwesome**: Used under the SIL OFL 1.1 License.
