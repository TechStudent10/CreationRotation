//   CREATION ROTATION LOGGING """LIBRARY"""
//   by TechStudent10

namespace log {
    // https://stackoverflow.com/a/41407246
    export enum TextColor {
        White = "\x1b[37m",
        Red = "\x1b[31m",
        Green = "\x1b[32m",
        Yellow = "\x1b[33m",
        Blue = "\x1b[34m",
        Magenta = "\x1b[35m",
        Cyan = "\x1b[36m",
        RESET = "\x1b[0m"
    }

    export function log(
        level: string,
        contents: any,
        color?: TextColor
    ) {
        level = level.toUpperCase()
        const timeStr = new Date().toLocaleString()
    
        const baseStr = `[${timeStr}] [${level}]`.padEnd(32, " ")
    
        color = color ? color : TextColor.White

        const actualContents = typeof contents === "string" ?
            contents :
            JSON.stringify(contents, null, 4)

        actualContents.split("\n").forEach((contents) => {
            console.log(`${color}${baseStr} ${contents}${TextColor.RESET}`)
        })
    }

    export function info(contents: any) {
        log(
            "INFO",
            contents
        )
    }

    export function debug(contents: any) {
        log(
            "DEBUG",
            contents,
            TextColor.Cyan
        )
    }

    export function error(contents: any) {
        log(
            "ERROR",
            contents,
            TextColor.Red
        )
    }

    export function packet(contents: any) {
        log(
            "PACKET",
            contents,
            TextColor.Yellow
        )
    }
}

export default log